#include "FeedbackSynthControls.h"
#include <functional>

using namespace infrasonic;
using namespace daisy;

void FeedbackSynth::Controls::Init(DaisyMargolis &hw, Engine &engine) {
    params_.Init(hw.AudioSampleRate() / hw.AudioBlockSize());
    //initADCs(hw);
    registerParams(engine);
}

void FeedbackSynth::Controls::Update(DaisyMargolis &hw) {
    params_.UpdateNormalized(Parameter::StringPitch,        hw.knob[hw.KNOB_1].Process());
    params_.UpdateNormalized(Parameter::FeedbackGain,       hw.knob[hw.KNOB_2].Process());
    params_.UpdateNormalized(Parameter::FeedbackDelay,      hw.knob[hw.KNOB_3].Process());
    params_.UpdateNormalized(Parameter::FeedbackLPFCutoff,  1.0f);
    params_.UpdateNormalized(Parameter::FeedbackHPFCutoff,  hw.knob[hw.KNOB_4].Process());
    params_.UpdateNormalized(Parameter::EchoDelayTime,      hw.knob[hw.KNOB_5].Process());
    params_.UpdateNormalized(Parameter::EchoDelayFeedback,  hw.knob[hw.KNOB_6].Process());
    params_.UpdateNormalized(Parameter::EchoDelaySend,      hw.knob[hw.KNOB_7].Process());
}



void FeedbackSynth::Controls::registerParams(Engine &engine) {
    using namespace std::placeholders;

    // Pitch as nn
    params_.Register(Parameter::StringPitch, 40.0f, 16.0f, 72.0f, std::bind(&Engine::SetStringPitch, &engine, _1), 0.2f);

    // Feedback Gain in dbFS
    params_.Register(Parameter::FeedbackGain, -60.0f, -60.0f, 12.0f, std::bind(&Engine::SetFeedbackGain, &engine, _1));

    // Feedback delay in seconds
    params_.Register(Parameter::FeedbackDelay, 0.001f, 0.001f, 0.1f, std::bind(&Engine::SetFeedbackDelay, &engine, _1), 1.0f, daisysp::Mapping::EXP);

    // Feedback filter cutoff in hz
    params_.Register(Parameter::FeedbackLPFCutoff, 18000.0f, 100.0f, 18000.0f, std::bind(&Engine::SetFeedbackLPFCutoff, &engine, _1), 0.05f, daisysp::Mapping::LOG);
    params_.Register(Parameter::FeedbackHPFCutoff, 250.0f, 10.0f, 4000.0f, std::bind(&Engine::SetFeedbackHPFCutoff, &engine, _1), 0.05f, daisysp::Mapping::LOG);

    // Echo Delay time in s
    params_.Register(Parameter::EchoDelayTime, 0.5f, 0.05f, 5.0f, std::bind(&Engine::SetEchoDelayTime, &engine, _1), 0.1f, daisysp::Mapping::EXP);

    // Echo Delay feedback
    params_.Register(Parameter::EchoDelayFeedback, 0.0f, 0.0f, 1.5f, std::bind(&Engine::SetEchoDelayFeedback, &engine, _1));

    // Echo Delay send
    params_.Register(Parameter::EchoDelaySend, 0.0f, 0.0f, 1.0f, std::bind(&Engine::SetEchoDelaySendAmount, &engine, _1), 0.05f, daisysp::Mapping::EXP);
}
