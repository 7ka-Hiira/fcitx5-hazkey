#include "azookey.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace fcitx
{

  static const std::array<Key, 10> selectionKeys = {
      Key{FcitxKey_1},
      Key{FcitxKey_2},
      Key{FcitxKey_3},
      Key{FcitxKey_4},
      Key{FcitxKey_5},
      Key{FcitxKey_6},
      Key{FcitxKey_7},
      Key{FcitxKey_8},
      Key{FcitxKey_9},
      Key{FcitxKey_0},
  };

  azooKeyEngine::azooKeyEngine(Instance *instance)
      : instance_(instance), factory_([this](InputContext &ic)
                                      { return new azooKeyState(&ic, this); })
  {
    kkc_config_ = kkc_get_config();
    instance->inputContextManager().registerProperty("azookeyState", &factory_);
  }

  void azooKeyEngine::keyEvent(const InputMethodEntry &entry,
                               KeyEvent &keyEvent)
  {
    FCITX_UNUSED(entry);
    if (keyEvent.isRelease() || keyEvent.key().states())
    {
      return;
    }
    FCITX_INFO() << "Surrounding: " << keyEvent.inputContext()->surroundingText();
    FCITX_INFO() << "Key: " << keyEvent.key().toString();
    auto *state = keyEvent.inputContext()->propertyFor(&factory_);
    state->keyEvent(keyEvent);
  }

  void azooKeyEngine::reset(const InputMethodEntry &,
                            InputContextEvent &event)
  {
    auto *state = event.inputContext()->propertyFor(&factory_);
    state->reset();
  }

  void azooKeyState::keyEvent(KeyEvent &event)
  {
    if (!buffer_.empty())
    {
      if (event.key().check(FcitxKey_BackSpace))
      {
        buffer_.backspace();
        updateUI();
        return event.filterAndAccept();
      }
      if (event.key().check(FcitxKey_Return))
      {
        ic_->commitString(event.inputContext()->inputPanel().clientPreedit().toStringForCommit());
        reset();
        buffer_.clear();
        kanjiMode = false;
        return event.filterAndAccept();
      }
      if (event.key().check(FcitxKey_Shift_L) ||
          event.key().check(FcitxKey_Shift_R))
      {
        kanjiMode = true;
        return event.filterAndAccept();
      }
      if (event.key().check(FcitxKey_Escape))
      {
        reset();
        kanjiMode = false;
        return event.filterAndAccept();
      }
      if (!event.key().isSimple())
      {
        FCITX_INFO() << "Complex key: " << event.key().sym();
        return event.filterAndAccept();
      }
    }
    else
    {
      if (!event.key().isSimple())
      {
        return;
      }
    }

    buffer_.type(event.key().sym());
    updateUI();
    return event.filterAndAccept();
  }

  void azooKeyState::updateUI()
  {
    auto &inputPanel = ic_->inputPanel();
    inputPanel.reset();
    if (!buffer_.empty())
    {
      char *hiragana = kkc_ascii2hiragana(buffer_.userInput().c_str(),
                                          engine_->getKkcConfig());

      char **converted = kkc_convert(hiragana, engine_->getKkcConfig());
      kkc_free_ascii2hiragana(hiragana);
      std::vector<std::string> candidatesStr;
      for (int i = 0; converted[i] != nullptr; i++)
      {
        candidatesStr.push_back(converted[i]);
      }

      inputPanel.setCandidateList(std::make_unique<azooKeyCandidateList>(
          engine_, ic_, &candidatesStr));

      if (ic_->capabilityFlags().test(CapabilityFlag::Preedit))
      {
        fcitx::Text preedit(*converted, fcitx::TextFormatFlag::HighLight);
        inputPanel.setClientPreedit(preedit);
      }
      else
      {
        fcitx::Text preedit(*converted);
        inputPanel.setPreedit(preedit);
      }
      kkc_free_convert(converted);
    }
    ic_->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
    ic_->updatePreedit();
  }

  azooKeyCandidateList::azooKeyCandidateList(azooKeyEngine *engine, InputContext *ic,
                         std::vector<std::string> *candidatesStr)
        : engine_(engine), ic_(ic), candidates_{}
    {
      setPageable(this);
      setCursorMovable(this);

      totalSize_ = candidatesStr->size();
      size_ = std::min(totalSize_, 10);
      totalPage_ = size_ / 10;
      for (int i = 0; i < size_; i++)
      {
        Text label;
        label.append(std::to_string((i + 1) % 10));
        label.append(". ");
        candidates_.emplace_back(std::make_unique<azooKeyCandidateWord>(engine_, candidatesStr->at(i)));
        labels_.push_back(label);
      }
    }

  FCITX_ADDON_FACTORY(azooKeyEngineFactory);

} // namespace fcitx