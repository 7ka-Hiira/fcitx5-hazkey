#ifndef _FCITX5_AZOOKEY_AZOOKEY_H_
#define _FCITX5_AZOOKEY_AZOOKEY_H_

#include <azookey/libazookey_kkc.h>
#include <fcitx-utils/key.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>
#include <iconv.h>

namespace fcitx {

class azooKeyEngine;

class azooKeyState : public InputContextProperty {
 public:
  azooKeyState(InputContext *ic, azooKeyEngine *engine)
      : engine_(engine), ic_(ic) {
    composingText_ = nullptr;
  }

  void keyEvent(KeyEvent &keyEvent);
  void candidateKeyEvent(KeyEvent &keyEvent,
                         std::shared_ptr<CommonCandidateList> candidateList);
  void preeditKeyEvent(
      KeyEvent &keyEvent,
      std::shared_ptr<CommonCandidateList> PreeditCandidateList);

  bool isShowingPreedit() const { return composingText_ != nullptr; }
  bool isCandidateMode() const { return isCandidateMode_; }

  void showCandidateList();
  void showPredictCandidateList();
  void hideList();
  void updateUI();
  void reset() {
    if (composingText_ != nullptr) {
      kkc_free_composing_text(composingText_);
    }
    composingText_ = nullptr;
    isCandidateMode_ = false;
    hideList();
    updateUI();
  }

 private:
  azooKeyEngine *engine_;
  InputContext *ic_;
  ComposingText *composingText_;
  bool isCandidateMode_;
  char *convertedPtr_;
  bool showingCandidateList_;
};

class azooKeyEngine : public InputMethodEngineV2 {
 public:
  azooKeyEngine(Instance *instance);
  void keyEvent(const InputMethodEntry &entry, KeyEvent &keyEvent) override;
  // void reset(const InputMethodEntry &, InputContextEvent &event) override;
  void activate(const InputMethodEntry &, InputContextEvent &) override;
  void deactivate(const InputMethodEntry &, InputContextEvent &) override;

  auto factory() const { return &factory_; }
  auto activate() const { return true; }
  auto instance() const { return instance_; }
  auto getKkcConfig() const { return kkc_config_; }

 private:
  Instance *instance_;
  FactoryFor<azooKeyState> factory_;
  iconv_t conv_;
  const KkcConfig *kkc_config_;
};

class azooKeyEngineFactory : public AddonFactory {
  AddonInstance *create(AddonManager *manager) override {
    FCITX_UNUSED(manager);
    return new azooKeyEngine(manager->instance());
  }
};

}  // namespace fcitx
#endif  // _FCITX5_AZOOKEY_AZOOKEY_H_
