#include <Geode/Geode.hpp>
#include <Geode/modify/GameLevelManager.hpp>
#include <Geode/modify/GJAccountManager.hpp>
#include <Geode/modify/GJMultiplayerManager.hpp>
#include <Geode/modify/MusicDownloadManager.hpp>

using namespace geode::prelude;

class CaptchaPopup : public Popup {
protected:
    CCMenuItemSpriteExtra* m_checkButton = nullptr;
    LoadingCircle* m_loadingCircle = nullptr;
    CCLabelBMFont* m_label = nullptr;
    bool m_clicked = false;

    bool setup() override {
        this->setTitle("CAPTCHA");

        m_label = CCLabelBMFont::create("I am not a robot", "bigFont.fnt");
        m_label->setPosition(ccp(185, 100));
        m_label->setScale(0.6f);
        m_mainLayer->addChild(m_label);

        auto checkButton = Button::create("GJ_checkOff_001.png", [this](auto) {
            this->onCheckClicked();
        });
        checkButton->setPosition(ccp(60, 100));
        m_buttonMenu->addChild(checkButton);
        m_checkButton = checkButton;

        return true;
    }

    void onCheckClicked() {
        if (m_clicked) return;
        m_clicked = true;

        m_checkButton->removeFromParent();
        
        m_loadingCircle = LoadingCircle::create();
        m_loadingCircle->setPosition(ccp(60, 100));
        m_loadingCircle->show();
        m_mainLayer->addChild(m_loadingCircle);

        this->scheduleOnce(schedule_selector(CaptchaPopup::onAutoCheck), 1.0f);
    }

    void onAutoCheck(float) {
        if (m_loadingCircle) {
            m_loadingCircle->fadeAndRemove();
        }

        auto onSprite = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
        onSprite->setPosition(ccp(60, 100));
        m_mainLayer->addChild(onSprite);

        this->scheduleOnce(schedule_selector(CaptchaPopup::closePopup), 0.5f);
    }

    void closePopup(float) {
        this->onClose(nullptr);
    }

public:
    static CaptchaPopup* create() {
        auto ret = new CaptchaPopup();
        if (ret->initAnchored(300.f, 200.f)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

template <typename T, typename Func>
void handleHttpRequest(T* self, Func originalFunc, gd::string url, gd::string params, gd::string tag, GJHttpType type) {
    auto chance = Mod::get()->getSettingValue<int64_t>("chance");
    if (chance > 0 && utils::random::chance(chance)) {
        CaptchaPopup::create()->show();
    }
    (self->*originalFunc)(url, params, tag, type);
}

class $modify(HookedGameLevelManager, GameLevelManager) {
    void ProcessHttpRequest(gd::string url, gd::string params, gd::string tag, GJHttpType type) {
        handleHttpRequest(this, &GameLevelManager::ProcessHttpRequest, url, params, tag, type);
    }
};

class $modify(HookedGJAccountManager, GJAccountManager) {
    void ProcessHttpRequest(gd::string url, gd::string params, gd::string tag, GJHttpType type) {
        handleHttpRequest(this, &GJAccountManager::ProcessHttpRequest, url, params, tag, type);
    }
};

class $modify(HookedGJMultiplayerManager, GJMultiplayerManager) {
    void ProcessHttpRequest(gd::string url, gd::string params, gd::string tag, GJHttpType type) {
        handleHttpRequest(this, &GJMultiplayerManager::ProcessHttpRequest, url, params, tag, type);
    }
};

class $modify(HookedMusicDownloadManager, MusicDownloadManager) {
    void ProcessHttpRequest(gd::string url, gd::string params, gd::string tag, GJHttpType type) {
        handleHttpRequest(this, &MusicDownloadManager::ProcessHttpRequest, url, params, tag, type);
    }
};
