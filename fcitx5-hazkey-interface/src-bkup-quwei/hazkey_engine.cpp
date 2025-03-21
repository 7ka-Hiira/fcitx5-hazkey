#include "hazkey_engine.h"

namespace {

// Template to help resolve iconv parameter issue on BSD.
template <class T>
struct function_traits;

// partial specialization for function pointer
template <class R, class... Args>
struct function_traits<R (*)(Args...)> {
    using result_type = R;
    using argument_types = std::tuple<Args...>;
};

template <class T>
using second_argument_type = typename std::tuple_element<
    1, typename function_traits<T>::argument_types>::type;

static const std::array<fcitx::Key, 10> selectionKeys = {
    fcitx::Key{FcitxKey_1}, fcitx::Key{FcitxKey_2}, fcitx::Key{FcitxKey_3},
    fcitx::Key{FcitxKey_4}, fcitx::Key{FcitxKey_5}, fcitx::Key{FcitxKey_6},
    fcitx::Key{FcitxKey_7}, fcitx::Key{FcitxKey_8}, fcitx::Key{FcitxKey_9},
    fcitx::Key{FcitxKey_0},
};

class HazkeyCandidateWord : public fcitx::CandidateWord {
public:
    HazkeyCandidateWord(HazkeyEngine *engine, std::string text)
        : engine_(engine) {
        setText(fcitx::Text(std::move(text)));
    }

    void select(fcitx::InputContext *inputContext) const override {
        inputContext->commitString(text().toString());
        auto state = inputContext->propertyFor(engine_->factory());
        state->reset();
    }

private:
    HazkeyEngine *engine_;
};

class HazkeyCandidateList : public fcitx::CandidateList,
                           public fcitx::PageableCandidateList,
                           public fcitx::CursorMovableCandidateList {
public:
    HazkeyCandidateList(HazkeyEngine *engine, fcitx::InputContext *ic,
                       const std::string &code)
        : engine_(engine), ic_(ic), code_(std::stoi(code)) {
        setPageable(this);
        setCursorMovable(this);
        for (int i = 0; i < 10; i++) {
            const char label[2] = {static_cast<char>('0' + (i + 1) % 10), '\0'};
            labels_[i].append(label);
            labels_[i].append(". ");
        }
        generate();
    }

    const fcitx::Text &label(int idx) const override { return labels_[idx]; }

    const fcitx::CandidateWord &candidate(int idx) const override {
        return *candidates_[idx];
    }
    int size() const override { return 10; }
    fcitx::CandidateLayoutHint layoutHint() const override {
        return fcitx::CandidateLayoutHint::NotSet;
    }
    bool usedNextBefore() const override { return false; }
    void prev() override {
        if (!hasPrev()) {
            return;
        }
        --code_;
        auto state = ic_->propertyFor(engine_->factory());
        state->setCode(code_);
    }
    void next() override {
        if (!hasNext()) {
            return;
        }
        code_++;
        auto state = ic_->propertyFor(engine_->factory());
        state->setCode(code_);
    }

    bool hasPrev() const override { return code_ > 0; }

    bool hasNext() const override { return code_ < 999; }

    void prevCandidate() override { cursor_ = (cursor_ + 9) % 10; }

    void nextCandidate() override { cursor_ = (cursor_ + 1) % 10; }

    int cursorIndex() const override { return cursor_; }

private:
    void generate() {
        for (int i = 0; i < 10; i++) {
            auto code = code_ * 10 + (i + 1);
            auto qu = code / 100;
            auto wei = code % 100;

            // Hazkey to GB2312 (0xA0 + qu, 0xA0 + wei)
            char in[3];
            if (qu >= 95) { /* Process extend Qu 95 and 96 */
                in[0] = qu - 95 + 0xA8;
                in[1] = wei + 0x40;

                /* skip 0xa87f and 0xa97f */
                if (in[1] >= 0x7f) {
                    in[1]++;
                }
            } else {
                in[0] = qu + 0xa0;
                in[1] = wei + 0xa0;
            }

            size_t insize = 2, avail = 10 + 1;
            std::remove_pointer_t<second_argument_type<decltype(&::iconv)>>
                inbuf = in;

            char out[10 + 1];
            char *outbuf = out;
            // iconv(engine_->conv(), &inbuf, &insize, &outbuf, &avail);
            *outbuf = '\0';
            candidates_[i] = std::make_unique<HazkeyCandidateWord>(engine_, out);
        }
    }

    HazkeyEngine *engine_;
    fcitx::InputContext *ic_;
    fcitx::Text labels_[10];
    std::unique_ptr<HazkeyCandidateWord> candidates_[10];
    int code_;
    int cursor_ = 0;
};

} // namespace

void HazkeyState::keyEvent(fcitx::KeyEvent &event) {
    int a = fcitx5Hazkey::hellohazkey();

    int b = fcitx5Hazkey::keyEvent(event.key());

    // if (auto candidateList = ic_->inputPanel().candidateList()) {
    //     int idx = event.key().keyListIndex(selectionKeys);
    //     if (idx >= 0 && idx < candidateList->size()) {
    //         event.accept();
    //         candidateList->candidate(idx).select(ic_);
    //         return;
    //     }
    //     if (event.key().checkKeyList(
    //             engine_->instance()->globalConfig().defaultPrevPage())) {
    //         if (auto *pageable = candidateList->toPageable();
    //             pageable && pageable->hasPrev()) {
    //             event.accept();
    //             pageable->prev();
    //             ic_->updateUserInterface(
    //                 fcitx::UserInterfaceComponent::InputPanel);
    //         }
    //         return event.filterAndAccept();
    //     }

    //     if (event.key().checkKeyList(
    //             engine_->instance()->globalConfig().defaultNextPage())) {
    //         if (auto *pageable = candidateList->toPageable();
    //             pageable && pageable->hasNext()) {
    //             pageable->next();
    //             ic_->updateUserInterface(
    //                 fcitx::UserInterfaceComponent::InputPanel);
    //         }
    //         return event.filterAndAccept();
    //     }
    // }

    // if (buffer_.empty()) {
    //     if (!event.key().isDigit()) {
    //         return;
    //     }
    // } else {
    //     if (event.key().check(FcitxKey_BackSpace)) {
    //         buffer_.backspace();
    //         updateUI();
    //         return event.filterAndAccept();
    //     }
    //     if (event.key().check(FcitxKey_Return)) {
    //         ic_->commitString(buffer_.userInput());
    //         reset();
    //         return event.filterAndAccept();
    //     }
    //     if (event.key().check(FcitxKey_Escape)) {
    //         reset();
    //         return event.filterAndAccept();
    //     }
    //     if (!event.key().isDigit()) {
    //         return event.filterAndAccept();
    //     }
    // }

    buffer_.type(event.key().sym());
    updateUI();
    return event.filterAndAccept();
}

void HazkeyState::setCode(int code) {
    if (code < 0 || code > 999) {
        return;
    }
    buffer_.clear();
    auto codeStr = std::to_string(code);
    while (codeStr.size() < 3) {
        codeStr = "0" + codeStr;
    }
    buffer_.type(std::to_string(code));
    updateUI();
}

void HazkeyState::updateUI() {
    auto &inputPanel = ic_->inputPanel();
    inputPanel.reset();
    if (buffer_.size() == 3) {
        inputPanel.setCandidateList(std::make_unique<HazkeyCandidateList>(
            engine_, ic_, buffer_.userInput()));
    }
    if (ic_->capabilityFlags().test(fcitx::CapabilityFlag::Preedit)) {
        fcitx::Text preedit(buffer_.userInput(),
                            fcitx::TextFormatFlag::HighLight);
        inputPanel.setClientPreedit(preedit);
    } else {
        fcitx::Text preedit(buffer_.userInput());
        inputPanel.setPreedit(preedit);
    }
    ic_->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
    ic_->updatePreedit();
}

HazkeyEngine::HazkeyEngine(fcitx::Instance *instance)
    : instance_(instance), factory_([this](fcitx::InputContext &ic) {
          return new HazkeyState(this, &ic);
      }) {
    conv_ = iconv_open("UTF-8", "GB18030");
    if (conv_ == reinterpret_cast<iconv_t>(-1)) {
        throw std::runtime_error("Failed to create converter");
    }
    instance->inputContextManager().registerProperty("hazkeyState", &factory_);
}

void HazkeyEngine::keyEvent(const fcitx::InputMethodEntry &entry,
                           fcitx::KeyEvent &keyEvent) {
    FCITX_UNUSED(entry);
    if (keyEvent.isRelease() || keyEvent.key().states()) {
        return;
    }
    // FCITX_INFO() << keyEvent.key() << " isRelease=" << keyEvent.isRelease();
    auto ic = keyEvent.inputContext();
    auto *state = ic->propertyFor(&factory_);
    state->keyEvent(keyEvent);
}

void HazkeyEngine::reset(const fcitx::InputMethodEntry &,
                        fcitx::InputContextEvent &event) {
    auto *state = event.inputContext()->propertyFor(&factory_);
    state->reset();
}

FCITX_ADDON_FACTORY(HazkeyEngineFactory);
