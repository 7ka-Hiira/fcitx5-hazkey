#ifndef _FCITX5_HAZKEY_HAZKEY_ENGINE_H_
#define _FCITX5_HAZKEY_HAZKEY_ENGINE_H_

#include <fcitx-config/iniparser.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>
#include <iconv.h>


#include <fcitx-utils/inputbuffer.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>



#include "../../fcitx5-hazkey/libfcitx5Hazkey.h"


    class HazkeyEngine;

    class HazkeyState : public fcitx::InputContextProperty {
    public:
        HazkeyState(HazkeyEngine *engine, fcitx::InputContext *ic)
            : engine_(engine), ic_(ic) {}
    
        void keyEvent(fcitx::KeyEvent &keyEvent);
        void setCode(int code);
        void updateUI();
        void reset() {
            buffer_.clear();
            updateUI();
        }
    
    private:
        HazkeyEngine *engine_;
        fcitx::InputContext *ic_;
        fcitx::InputBuffer buffer_{{fcitx::InputBufferOption::AsciiOnly,
                                    fcitx::InputBufferOption::FixedCursor}};
    };

class HazkeyEngine : public fcitx::InputMethodEngineV2 {
    public:
    HazkeyEngine(fcitx::Instance *instance);

    void keyEvent(const fcitx::InputMethodEntry &entry,
                  fcitx::KeyEvent &keyEvent) override;

    void reset(const fcitx::InputMethodEntry &,
               fcitx::InputContextEvent &event) override;

    auto factory() const { return &factory_; }
    auto conv() const { return conv_; }
    auto instance() const { return instance_; }

private:
    fcitx::Instance *instance_;
    fcitx::FactoryFor<HazkeyState> factory_;
    iconv_t conv_;
};

class HazkeyEngineFactory : public fcitx::AddonFactory {
    fcitx::AddonInstance *create(fcitx::AddonManager *manager) override {
        FCITX_UNUSED(manager);
        return new HazkeyEngine(manager->instance());
    }
};

#endif  // _FCITX5_HAZKEY_HAZKEY_ENGINE_H_
