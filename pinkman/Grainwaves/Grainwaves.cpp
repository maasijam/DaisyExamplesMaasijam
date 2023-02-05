#include "../daisy_pinkman.h"
#include "daisysp.h"
#include "core_cm7.h"
#include "utils.h"


using namespace daisy;
using namespace pinkman;
using namespace daisysp;
using namespace std;

enum Leds {
  LED_1,
  LED_2,
  LED_3,
  LED_4,
  LED_5,
  LED_LAST
};

#define RECORDING_XFADE_OVERLAP 100 // Samples
#define RECORDING_BUFFER_SIZE (48000 * 5) // X seconds at 48kHz
#define MIN_GRAIN_SIZE 480 // 10 ms
#define MAX_GRAIN_SIZE (48000 * 2) // 2 second
#define MAX_GRAIN_COUNT 32
#define SHOW_PERFORMANCE_BARS true

struct Grain {
    int length = 0;
    int spawn_position = 0;
    int step = 0;
    float pan = 0; // 0 is left, 1 is right.
    float playback_speed = 0;
};

DaisyPinkman patch;
CpuLoadMeter cpu_load_meter;
Switch       record_button;
Switch       shift_button;
//ReverbSc     reverb;
Led led[LED_LAST];

CrossFade drywet;




float DSY_SDRAM_BSS recording[RECORDING_BUFFER_SIZE];
size_t recording_length = RECORDING_BUFFER_SIZE;
size_t write_head = 0;

const uint8_t MAX_SPAWN_POINTS = 7;

//const size_t RENDERABLE_RECORDING_BUFFER_SIZE = oled.width;
//const float RECORDING_TO_RENDERABLE_RECORDING_BUFFER_RATIO = RENDERABLE_RECORDING_BUFFER_SIZE / (float)RECORDING_BUFFER_SIZE;
//const float RENDERABLE_RECORDING_TO_RECORDING_BUFFER_RATIO = RECORDING_BUFFER_SIZE / (float)RENDERABLE_RECORDING_BUFFER_SIZE;
//float DSY_SDRAM_BSS renderable_recording[RENDERABLE_RECORDING_BUFFER_SIZE]; // Much lower resolution, for easy rendering
//size_t last_written_renderable_recording_index = 0; 

bool is_recording = true;
bool is_stopping_recording = false;
size_t recording_xfade_step = 0;
bool is_tracking = true;

float spawn_position_scan_speed;
float spawn_position_offset;
float spawn_positions_splay;
float spawn_positions_count;
float pitch_shift_in_semitones;
int grain_length;
float grain_density; // Target concurrent grains
unsigned int spawn_time; // The number of samples between each new grain
float spawn_time_spread; // The variance of the spawn rate
uint32_t last_spawn_time_at_position[MAX_SPAWN_POINTS] = { 0 };
const int SPAWN_BAR_FLASH_MILLIS = 250;

int next_spawn_position_index = 0;
float spawn_position = 0.f;
float next_spawn_offset;
uint32_t samples_since_last_spawn = 0;

Grain grains[MAX_GRAIN_COUNT];
Stack<uint8_t, MAX_GRAIN_COUNT> available_grains;

// Responsible for wrapping the index
inline float get_sample(int index) {
    return recording[wrap(index, 0, recording_length)];
}

inline size_t get_spawn_position(int index) {
    int unwrapped_spawn_position;
    
    if (is_tracking) {
        unwrapped_spawn_position = write_head + (spawn_position_offset - (recording_length / 2));
    } else {
        unwrapped_spawn_position = spawn_position_offset;
    }
                    
    if (index == (int)spawn_positions_count - 1) {
        unwrapped_spawn_position += spawn_positions_splay;
    } else if (index != 0) {
        unwrapped_spawn_position += index * (spawn_positions_splay / (spawn_positions_count - 2));
    }
    
    return fwrap(unwrapped_spawn_position, 0.f, recording_length);
}

inline void record_xfaded_sample(float sample_in) {
    float xfade_magnitude = (recording_xfade_step + 1) / ((float)RECORDING_XFADE_OVERLAP + 1.f);

    recording[write_head] = lerp(
        recording[write_head],
        sample_in,
        xfade_magnitude
    );
}

void AudioCallback(
    AudioHandle::InputBuffer  in,
    AudioHandle::OutputBuffer out,
    size_t size
) {
    cpu_load_meter.OnBlockStart();

    patch.ProcessAllControls();
    record_button.Debounce();
    shift_button.Debounce();

    spawn_position_offset = patch.GetAdcValue(POT_8) * recording_length;
    spawn_positions_count = 2.7f + map_to_range(patch.GetAdcValue(POT_6), 0, MAX_SPAWN_POINTS - 2);

    // Deadzone at 0 to the spawn position splay
    float raw_spawn_pos_splay = map_to_range(patch.GetAdcValue(POT_7), -1, 1);
    if (raw_spawn_pos_splay < -0.05f) {
        raw_spawn_pos_splay = raw_spawn_pos_splay + 0.05f;
    } else if (raw_spawn_pos_splay > 0.05f) {
        raw_spawn_pos_splay = raw_spawn_pos_splay - 0.05f;
    } else {
        raw_spawn_pos_splay = 0;
        spawn_positions_count = 1;
    }
    spawn_positions_splay = raw_spawn_pos_splay * recording_length;

    // pitch_shift_in_semitones = map_to_range(patch.GetAdcValue(CV_7), -12 * 5, 12 * 5); // volt per octave
    pitch_shift_in_semitones = map_to_range(patch.GetAdcValue(POT_4), -24, 24); // Without CV this is more playable

    grain_length = map_to_range(abs(patch.GetAdcValue(POT_2) - 0.5f) * 2, MIN_GRAIN_SIZE, MAX_GRAIN_SIZE);
    if (patch.GetAdcValue(POT_2) < 0.5f) {
        grain_length = -grain_length;
    }

    grain_density = map_to_range(patch.GetAdcValue(POT_1) * patch.GetAdcValue(POT_1), 0.5f, MAX_GRAIN_COUNT);
    spawn_time = abs(grain_length) / grain_density;
    spawn_time_spread = patch.GetAdcValue(POT_5);  
    float actual_spawn_time = spawn_time * (1 + next_spawn_offset * spawn_time_spread);

    //float reverb_amount = patch.GetAdcValue(POT_3);
    //float reverb_time = fmap(min(reverb_amount, 0.5f) * 2, 0.5f, 0.99f);
    //float reverb_damp = fmap(reverb_amount, 100.f, 24000.f, Mapping::LOG);
    //float reverb_wet_mix = min(reverb_amount, 0.1f) * 7;
    float mix_amount = patch.GetAdcValue(POT_3);
    drywet.SetPos(mix_amount);

    //reverb.SetFeedback(reverb_time);
    //reverb.SetLpFreq(reverb_damp);

    // Toggle the record state on button press
    if(record_button.RisingEdge())
    {
        if (!is_recording) {
            is_recording = true;
            recording_xfade_step = 0;
        } else {
            is_stopping_recording = true;
        }
    }

    is_tracking = shift_button.Pressed();

    //patch.SetLed(is_tracking); 
    led[LED_3].Set(is_tracking ? 1.f : 0.f);
    led[LED_3].Update();

    // Process audio
    for(size_t i = 0; i < size; i++)
    {
        if (is_recording) {
            if (is_stopping_recording) {
                // Record a little extra at the end of the recording so we can xfade the values
                // and stop the pop sound
                record_xfaded_sample(IN_L[i]);

                recording_xfade_step--;

                if (recording_xfade_step == 0) {
                    is_recording = false;
                    is_stopping_recording = false;
                }
            } else if (recording_xfade_step < RECORDING_XFADE_OVERLAP) {
                // xfade in the start of the recording to stop the pop sound
                record_xfaded_sample(IN_L[i]);

                recording_xfade_step++;
            } else {
                recording[write_head] = IN_L[i];
            }

            // TODO: Record positive and negative values seperately
            //size_t renderable_recording_index = write_head * RECORDING_TO_RENDERABLE_RECORDING_BUFFER_RATIO;

            // Clear out the element when we first start writing fresh values to it
            //if (write_head == 0 || renderable_recording_index > last_written_renderable_recording_index) {
            //    renderable_recording[renderable_recording_index] = 0;
            //}

            // Downsample the samples into renderable_recording_index by averaging them
            //renderable_recording[renderable_recording_index] += abs(IN_L[i]) / RENDERABLE_RECORDING_TO_RECORDING_BUFFER_RATIO;
            //last_written_renderable_recording_index = renderable_recording_index;
            
            if (recording_length < RECORDING_BUFFER_SIZE) {
                recording_length++;
            }
        }

        write_head++;
        if (write_head >= RECORDING_BUFFER_SIZE) {
            write_head = 0;
        }

        if (recording_length > 4800 /* A kinda abitrary number */) {
            samples_since_last_spawn++;

            // Spawn grains
            if (samples_since_last_spawn >= actual_spawn_time && !available_grains.IsEmpty()) {
                samples_since_last_spawn = 0;

                size_t new_grain_index = available_grains.PopBack();

                grains[new_grain_index].length = abs(grain_length);
                grains[new_grain_index].step = 0;
                grains[new_grain_index].pan = 0.5f + randF(-0.5f, 0.5f);
                
                grains[new_grain_index].spawn_position = get_spawn_position(next_spawn_position_index);
                last_spawn_time_at_position[next_spawn_position_index] = System::GetNow();
            
                float pitch_shift_in_octaves = pitch_shift_in_semitones / 12.f;

                // Reverse the playback if the length is negative
                if (grain_length > 0) {
                    grains[new_grain_index].playback_speed = pow(2, pitch_shift_in_octaves);
                } else {
                    grains[new_grain_index].playback_speed = -pow(2, pitch_shift_in_octaves);
                }

                next_spawn_offset = randF(-1.f, 1.f); // +/- 100%
                next_spawn_position_index = wrap(next_spawn_position_index + 1, 0, (int)spawn_positions_count);
            }

            // Calculate output
            float wet_l = 0.f;
            float wet_r = 0.f;

            for (int j = 0; j < MAX_GRAIN_COUNT; j++) {
                if (grains[j].step <= grains[j].length) {
                    size_t buffer_index = grains[j].spawn_position + grains[j].step * grains[j].playback_speed;

                    // playback_speed is a float so we need to interpolate between samples
                    float sample = get_sample(buffer_index);
                    float next_sample = get_sample(buffer_index + 1);

                    float decimal_portion = modf(grains[j].step * grains[j].playback_speed);
                    float interpolated_sample = sample * (1 - decimal_portion) + next_sample * decimal_portion;

                    // hacky bad envelope
                    float envelope_mult = min((grains[j].length - grains[j].step), grains[j].step) / max(1.f, (float)grains[j].length);
                    float signal = interpolated_sample * envelope_mult;

                    wet_l += (1.f - grains[j].pan) * signal;
                    wet_r += grains[j].pan * signal;

                    grains[j].step++;

                    if (grains[j].step > grains[j].length) {
                        available_grains.PushBack(j);
                    }
                }
            }

            //float reverb_in_l = wet_l;
            //float reverb_in_r = wet_r;
            //float reverb_wet_l, reverb_wet_r;
            //reverb.Process(reverb_in_l, reverb_in_r, &reverb_wet_l, &reverb_wet_r);
            //OUT_L[i] = reverb_in_l + (reverb_wet_l * reverb_wet_mix) + IN_L[i];
            //OUT_R[i] = reverb_in_r + (reverb_wet_r * reverb_wet_mix) + IN_L[i];
            float orig_l = IN_L[i];
            float orig_r = IN_L[i];
            float mix_left_SUM = drywet.Process(orig_l,wet_l);    //mix to mono if width 0.0
            float mix_right_SUM = drywet.Process(orig_r,wet_r);

            //OUT_L[i] = reverb_in_l + (reverb_wet_l * reverb_wet_mix);
            //OUT_R[i] = reverb_in_r + (reverb_wet_r * reverb_wet_mix);
            OUT_L[i] = mix_left_SUM;
            OUT_R[i] = mix_right_SUM;
        } else {
            OUT_L[i] = 0;
            OUT_R[i] = 0;
        }
    }

    cpu_load_meter.OnBlockEnd();
}

uint32_t last_render_millis = 0;
uint32_t last_debug_print_millis = 0;

int main(void)
{
    patch.Init();
    record_button.Init(patch.B7);
    shift_button.Init(patch.B8);
    patch.StartLog();

    drywet.Init();
    drywet.SetCurve(CROSSFADE_CPOW);

    // Populate available grains stack
    for (u_int8_t i = 0; i < MAX_GRAIN_COUNT; i++) {
        available_grains.PushBack(i);
    }

    dsy_gpio_pin led_pins[LED_LAST] = {patch.C10,patch.D8,patch.D9,patch.B5,patch.B6};
    for (size_t i = 0; i < LED_LAST; i++)
    {
        led[i].Init(led_pins[i],false);
    }

    // Clear the buffers
    //memset(renderable_recording, 0, sizeof(renderable_recording));
    memset(recording, 0, sizeof(recording));

    cpu_load_meter.Init(patch.AudioSampleRate(), patch.AudioBlockSize());
    //reverb.Init(patch.AudioSampleRate());
    patch.StartAudio(AudioCallback);

    while(1) {
        
    }
} 
 