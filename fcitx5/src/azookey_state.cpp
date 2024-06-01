#include "azookey_state.h"

#include <algorithm>
#include <numeric>

#include "azookey_candidate.h"
#include "azookey_config.h"

namespace fcitx {

constexpr int NormalCandidateListNBest = 9;
constexpr int PredictCandidateListNBest = 4;

void azooKeyState::keyEvent(KeyEvent &event) {
  FCITX_DEBUG() << "azooKeyState keyEvent";

  auto candidateList = std::dynamic_pointer_cast<azooKeyCandidateList>(
      event.inputContext()->inputPanel().candidateList());
  if (isCandidateMode_) {
    candidateKeyEvent(event, candidateList);
  } else if (composingText_ != nullptr) {
    preeditKeyEvent(event, candidateList);
  } else {
    auto key = event.key();
    if (!key.check(FcitxKey_Return) && !key.check(FcitxKey_space) &&
        key.isSimple()) {
      composingText_ = kkc_get_composing_text_instance();
      kkc_input_text(composingText_, Key::keySymToUTF8(key.sym()).c_str());
      updateOnPreeditMode();
    } else {
      return event.filter();
    }
    updateUI();
    return event.filterAndAccept();
  }
}

void azooKeyState::preeditKeyEvent(
    KeyEvent &event,
    std::shared_ptr<azooKeyCandidateList> PreeditCandidateList) {
  FCITX_DEBUG() << "azooKeyState preeditKeyEvent";

  auto key = event.key();
  auto keysym = key.sym();
  switch (keysym) {
    case FcitxKey_Return:
      ic_->commitString(event.inputContext()
                            ->inputPanel()
                            .clientPreedit()
                            .toStringForCommit());
      reset();
      break;
    case FcitxKey_BackSpace:
      kkc_delete_backward(composingText_);
      updateOnPreeditMode();
      break;
    case FcitxKey_Up:
    case FcitxKey_Tab:
      PreeditCandidateList->setDefaultStyle(config_->getSelectionKeys());
      setCandidateCursorAUX(PreeditCandidateList.get());
      isCandidateMode_ = true;
      break;
    case FcitxKey_Down:
      PreeditCandidateList->setDefaultStyle(config_->getSelectionKeys());
      advanceCandidateCursor(PreeditCandidateList.get());
      isCandidateMode_ = true;
      break;
    case FcitxKey_space:
      prepareNormalCandidateList();
      break;
    case FcitxKey_Escape:
      reset();
      break;
    case FcitxKey_period:
      ic_->commitString(event.inputContext()
                            ->inputPanel()
                            .clientPreedit()
                            .toStringForCommit());
      ic_->commitString("。");
      reset();
      break;
    case FcitxKey_comma:
      ic_->commitString(event.inputContext()
                            ->inputPanel()
                            .clientPreedit()
                            .toStringForCommit());
      ic_->commitString("、");
      reset();
      break;
    default:
      if (key.isSimple()) {
        kkc_input_text(composingText_, Key::keySymToUTF8(keysym).c_str());
        updateOnPreeditMode();
      }
      break;
  }
  updateUI();
  return event.filterAndAccept();
}

void azooKeyState::candidateKeyEvent(
    KeyEvent &event, std::shared_ptr<azooKeyCandidateList> candidateList) {
  FCITX_DEBUG() << "azooKeyState candidateKeyEvent";

  auto key = event.key();
  auto keysym = key.sym();
  std::vector<std::string> preedit;
  switch (keysym) {
    case FcitxKey_Right:
    case FcitxKey_Return:
      preedit = candidateList->azooKeyCandidate(candidateList->cursorIndex())
                    .getPreedit();
      ic_->commitString(preedit[0]);
      if (preedit.size() > 1) {
        auto correspondingCount =
            candidateList->azooKeyCandidate(candidateList->cursorIndex())
                .correspondingCount();
        kkc_complete_prefix(composingText_, correspondingCount);
        prepareNormalCandidateList();
      } else {
        reset();
      }
      updateUI();
      return event.filterAndAccept();
      break;
    case FcitxKey_BackSpace:
      updateOnPreeditMode();
      break;
    case FcitxKey_space:
    case FcitxKey_Tab:
    case FcitxKey_Down:
      advanceCandidateCursor(candidateList.get());
      break;
    case FcitxKey_Up:
      backCandidateCursor(candidateList.get());
      break;
    case FcitxKey_Escape:
      reset();
      break;
    default:
      if (key.checkKeyList(config_->getSelectionKeys())) {
        auto index = key.keyListIndex(config_->getSelectionKeys());
        candidateList->candidate(index).select(ic_);
        reset();
      } else if (key.isSimple()) {
        kkc_input_text(composingText_, Key::keySymToUTF8(keysym).c_str());
      } else {
        return event.filter();
      }
      break;
  }
  updateUI();
  return event.filterAndAccept();
}

void azooKeyState::prepareCandidateList(bool isPredictMode, bool updatePreedit,
                                        int nBest) {
  FCITX_DEBUG() << "azooKeyState prepareCandidateList";
  if (composingText_ == nullptr) {
    return;
  }

  auto ***candidates = kkc_get_candidates(
      composingText_, config_->getKkcConfig(), isPredictMode, nBest);

  std::vector<std::string> preeditSegments = {};
  bool preeditFound = false;

  auto candidateList = std::make_unique<azooKeyCandidateList>();
  for (int i = 0; candidates[i] != nullptr; i++) {
    std::vector<std::string> parts;
    std::vector<int> partLens;

    for (int j = 5;
         (candidates[i][j] != nullptr && candidates[i][j + 1] != nullptr);
         j += 2) {
      parts.push_back(candidates[i][j]);
      partLens.push_back(atoi(candidates[i][j + 1]));
    }

    // append words to the candidate list
    candidateList->append(std::make_unique<azooKeyCandidateWord>(
        candidates[i][0], candidates[i][2], atoi(candidates[i][3]), parts,
        partLens));

    // get first non-predicting candidate for preedit
    if (updatePreedit && !preeditFound && strcmp(candidates[i][4], "1") == 0) {
      preeditSegments = parts;
      preeditFound = true;
    }
  }
  candidateList->setDefaultStyle(config_->getSelectionKeys());
  ic_->inputPanel().reset();
  if (isPredictMode) {
    candidateList->setPageSize(4);
    ic_->inputPanel().setAuxUp(Text("[Tabキーで選択]"));
  } else {
    candidateList->setDefaultStyle(config_->getSelectionKeys());
    setCandidateCursorAUX(candidateList.get());
    preedit_.setPreedit(candidateList->candidate(0).text());
  }

  if (updatePreedit && !preeditSegments.empty()) {
    preedit_.setMultiSegmentPreedit(preeditSegments, 0);
  } else if (updatePreedit) {
    auto hiragana = kkc_get_composing_hiragana(composingText_);
    preedit_.setSimplePreedit(hiragana);
    kkc_free_composing_hiragana(hiragana);
  }

  ic_->inputPanel().setCandidateList(std::move(candidateList));
  isCandidateMode_ = !isPredictMode;
  kkc_free_candidates(candidates);
}

void azooKeyState::prepareNormalCandidateList() {
  prepareCandidateList(false, true, NormalCandidateListNBest);
}

void azooKeyState::preparePredictCandidateList() {
  prepareCandidateList(true, true, PredictCandidateListNBest);
}

void azooKeyState::advanceCandidateCursor(azooKeyCandidateList *candidateList) {
  candidateList->nextCandidate();
  setCandidateCursorAUX(candidateList);
  auto text = candidateList->azooKeyCandidate(candidateList->cursorIndex())
                  .getPreedit();

  preedit_.setMultiSegmentPreedit(text, 0);
}

void azooKeyState::backCandidateCursor(azooKeyCandidateList *candidateList) {
  candidateList->prevCandidate();
  setCandidateCursorAUX(candidateList);
  auto text = candidateList->azooKeyCandidate(candidateList->cursorIndex())
                  .getPreedit();
  preedit_.setMultiSegmentPreedit(text, 0);
}

void azooKeyState::setCandidateCursorAUX(azooKeyCandidateList *candidateList) {
  auto label = "[" + std::to_string(candidateList->globalCursorIndex() + 1) +
               "/" + std::to_string(candidateList->totalSize()) + "]";
  ic_->inputPanel().setAuxUp(Text(label));
}

void azooKeyState::updateOnPreeditMode() {
  FCITX_DEBUG() << "azooKeyState updateOnPreeditMode";

  if (composingText_ == nullptr) {
    reset();
    return;
  }

  auto hiragana = kkc_get_composing_hiragana(composingText_);

  if (hiragana == nullptr || strlen(hiragana) == 0) {
    reset();
    return;
  }

  preparePredictCandidateList();
}

void azooKeyState::updateUI() {
  ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
  ic_->updatePreedit();
}

void azooKeyState::loadConfig(std::shared_ptr<azooKeyConfig> &config) {
  if (config_ == nullptr) {
    config_ = config;
  }
}

void azooKeyState::reset() {
  FCITX_DEBUG() << "azooKeyState reset";
  if (composingText_ != nullptr) {
    kkc_free_composing_text_instance(composingText_);
  }
  ic_->inputPanel().reset();
  composingText_ = nullptr;
  isCandidateMode_ = false;
}

}  // namespace fcitx