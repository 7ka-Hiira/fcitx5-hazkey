#ifndef LIBHAZKEY_H
#define LIBHAZKEY_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KkcConfig KkcConfig;

// style: 0: halfwidth, 1: fullwidth
// tenCombining: 0: normal, 1: combining
// autoCommitModeNum: 0: none, 1: commit on ".", 2: commit on ".!?",//
// 3: commit on ".,!?"
const KkcConfig *kkc_get_config(bool zenzaiEnabled, int zenzaiInferLimit,
                                int numberFullwidth, int symbolFullwidth,
                                int periodFullwidth, int commaFullwidth,
                                int spaceFullwidth, int diacriticStyle,
                                int gpuLayers, const char *profileTextPtr);
void kkc_free_config(const KkcConfig *kkcConfigPtr);

void kkc_set_left_context(const KkcConfig *kkcConfigPtr,
                          const char *surrowndingTextPtr, int anchorIndex);

struct ComposingText;
ComposingText *kkc_get_composing_text_instance(void);
void kkc_free_composing_text_instance(ComposingText *composingTextPtr);

void kkc_input_text(ComposingText *composingTextPtr,
                    const KkcConfig *kkcConfigPtr, const char *stringPtr,
                    bool isDirectInput);
void kkc_delete_backward(ComposingText *composingTextPtr);
void kkc_delete_forward(ComposingText *composingTextPtr);
void kkc_complete_prefix(ComposingText *composingTextPtr,
                         int correspondingCount);
int kkc_move_cursor(ComposingText *composingTextPtr, long long cursorIndex);

char *kkc_get_composing_hiragana(ComposingText *composingTextPtr);
char *kkc_get_composing_hiragana_with_cursor(ComposingText *composingTextPtr);
char *kkc_get_composing_katakana_fullwidth(ComposingText *composingTextPtr);
char *kkc_get_composing_katakana_halfwidth(ComposingText *composingTextPtr);

char *kkc_get_composing_alphabet_halfwidth(ComposingText *composingTextPtr,
                                           const char *currentPreeditPtr);
char *kkc_get_composing_alphabet_fullwidth(ComposingText *composingTextPtr,
                                           const char *currentPreeditPtr);
void *kkc_free_text(char *Ptr);

//
// return value:
// UnsafeMutablePointer<UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>?>?
//
// candidate: candidate text shown in list
// description: description of the candidate. unused now ("")
// subHiragana: hiragana for non-converted parts
// correspondingCount: corresponding key count of the candidate
// liveTextCompatible: whether the candidate has same length with input
// Part: One of the converted phrases
// PartLen: Length of ruby of the converted phrase
//
// Example of element array in the return value array:
// ---------------------------------
// | index | value                 |
// ---------------------------------
// | 0     | candidate             |
// | 1     | description           |
// | 2     | subHiragana           |
// | 3     | correspondingCount    |
// | 4     | liveTextCompatible    |
// | 5     | Part1                 |
// | 6     | PartLen1              |
// | 7     | Part2                 |
// | 8     | PartLen2              |
// | ...   | ...                   |
//
char ***kkc_get_candidates(ComposingText *composingTextPtr,
                           const KkcConfig *kkcConfigPtr, bool isPredictMode,
                           int nBest);
void kkc_free_candidates(char ***candidatesPtr);

#ifdef __cplusplus
}
#endif

#endif  // LIBHAZKEY_H