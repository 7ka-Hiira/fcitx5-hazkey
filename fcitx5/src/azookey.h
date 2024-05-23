#ifndef _FCITX5_AZOOKEY_AZOOKEY_H_
#define _FCITX5_AZOOKEY_AZOOKEY_H_

#include <fcitx-utils/inputbuffer.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/candidatelist.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>
#include <azookey/libazookey_kkc.h>
#include <iconv.h>

namespace fcitx
{

  class azooKeyEngine;

  class azooKeyState : public InputContextProperty
  {
  public:
    azooKeyState(InputContext *ic, azooKeyEngine *engine)
        : engine_(engine), ic_(ic) {}

    void keyEvent(KeyEvent &keyEvent);
    void updateUI();
    void reset()
    {
      buffer_.clear();
      updateUI();
    }
    bool isKanjiMode() const { return kanjiMode; }

  private:
    azooKeyEngine *engine_;
    InputContext *ic_;
    InputBuffer buffer_{{InputBufferOption::AsciiOnly,
                         InputBufferOption::FixedCursor}};
    bool kanjiMode = false;
  };

  class azooKeyEngine : public InputMethodEngineV2
  {
  public:
    azooKeyEngine(Instance *instance);
    void keyEvent(const InputMethodEntry &entry,
                  KeyEvent &keyEvent) override;

    void reset(const InputMethodEntry &,
               InputContextEvent &event) override;

    auto factory() const { return &factory_; }
    auto conv() const { return conv_; }
    auto instance() const { return instance_; }
    auto getKkcConfig() const { return kkc_config_; }

  private:
    Instance *instance_;
    FactoryFor<azooKeyState> factory_;
    iconv_t conv_;
    const KkcConfig *kkc_config_;
  };

  class azooKeyEngineFactory : public AddonFactory
  {
    AddonInstance *create(AddonManager *manager) override
    {
      FCITX_UNUSED(manager);
      return new azooKeyEngine(manager->instance());
    }
  };

  class azooKeyCandidateWord : public CandidateWord
  {
  public:
    azooKeyCandidateWord(azooKeyEngine *engine, std::string text)
        : engine_(engine)
    {
      setText(Text(std::move(text)));
    }

    void select(InputContext *ic) const override
    {
      auto *state = ic->propertyFor(engine_->factory());
      state->reset();
      state->updateUI();
    }

  private:
    azooKeyEngine *engine_;
    std::vector<std::string> candidates_;
  };

  class azooKeyCandidateList : public fcitx::CandidateList,
  public fcitx::PageableCandidateList,
  public fcitx::CursorMovableCandidateList
  {
  public:
    azooKeyCandidateList(azooKeyEngine *engine, InputContext *ic,
                         std::vector<std::string> *candidatesStr);

    const Text &label(int idx) const override { return labels_[idx]; }
    
    const CandidateWord &candidate(int idx) const override
    {
      return *candidates_[idx];
    }
    
    int size() const override { return size_; }
    CandidateLayoutHint layoutHint() const override
    {
      return fcitx::CandidateLayoutHint::NotSet;
    }

    // Todo
    bool hasNext() const override { return currentPage_ < totalPage_; }
    bool hasPrev() const override { return currentPage_ > 0; }
    void prev() override { return; } 
    void next()  override { return; } 
    bool usedNextBefore() const override { return false; } 
    void prevCandidate() override { return; } 
    void nextCandidate() override { return; } 
    int cursorIndex() const override { return cursor_; } 

  private:
    void generate(std::vector<std::string> *candidatesStr);

    azooKeyEngine *engine_;
    fcitx::InputContext *ic_;
    std::vector<Text> labels_;
    std::vector<std::unique_ptr<azooKeyCandidateWord>> candidates_;
    // std::array<std::string, 10> candidates_;
    int totalSize_ = 0;
    int size_ = 0;
    int currentPage_ = 0;
    int totalPage_ = 0;
    int cursor_ = 0;
  };

} // namespace fcitx
#endif // _FCITX5_AZOOKEY_AZOOKEY_H_
