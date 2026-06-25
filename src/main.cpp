#include <Geode/Geode.hpp>
#include <Geode/ui/LoadingSpinner.hpp>
#include <Geode/modify/GameLevelManager.hpp>
#include <Geode/modify/GJAccountManager.hpp>
#include <Geode/modify/GJMultiplayerManager.hpp>
#include <Geode/modify/MusicDownloadManager.hpp>
#include <cstdlib>

using namespace geode::prelude;

class CaptchaPopup : public Popup {
protected:
    static CaptchaPopup* s_currentPopup;
    CCMenuItemSpriteExtra* m_checkButton = nullptr;
    LoadingSpinner* m_loadingSpinner = nullptr;
    CCLabelBMFont* m_label = nullptr;
    CCMenu* m_menu = nullptr;
    bool m_clicked = false;

    bool init() override {
        if (!Popup::init(300.f, 100.f))
            return false;

        this->setTitle("CAPTCHA");

        m_label = CCLabelBMFont::create("I am not a robot", "bigFont.fnt");
        m_label->setPosition(ccp(185, 100));
        m_label->setScale(0.6f);
        m_mainLayer->addChild(m_label);

        auto offSprite = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        m_checkButton = CCMenuItemSpriteExtra::create(
            offSprite,
            this,
            menu_selector(CaptchaPopup::onCheckClicked)
        );
        m_checkButton->setPosition(ccp(60, 100));

        m_menu = CCMenu::create();
        m_menu->setPosition(0, 0);
        m_menu->addChild(m_checkButton);
        m_mainLayer->addChild(m_menu);

        return true;
    }

    void onCheckClicked(CCObject*) {
        if (m_clicked) return;
        m_clicked = true;

        m_label->setVisible(false);
        m_checkButton->setVisible(false);
        
        m_loadingSpinner = LoadingSpinner::create(40.f);
        m_loadingSpinner->setPosition(ccp(150, 100));
        m_mainLayer->addChild(m_loadingSpinner);

        this->scheduleOnce(schedule_selector(CaptchaPopup::onAutoCheck), 1.0f);
    }

    void onAutoCheck(float) {
        if (m_loadingSpinner) {
            m_loadingSpinner->removeFromParent();
        }

        m_label->setVisible(true);
        m_checkButton->setVisible(true);
        auto onSprite = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
        onSprite->setPosition(ccp(60, 100));
        m_mainLayer->addChild(onSprite);

        this->scheduleOnce(schedule_selector(CaptchaPopup::closePopup), 0.5f);
    }

    void closePopup(float) {
        this->onClose(nullptr);
    }

    void onClose(CCObject* sender) override {
        Popup::onClose(sender);
        s_currentPopup = nullptr;
    }

public:
    static CaptchaPopup* create() {
        auto ret = new CaptchaPopup();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    static bool isPopupVisible() {
        return s_currentPopup != nullptr;
    }

    static void showPopup() {
        if (s_currentPopup) return;
        auto popup = CaptchaPopup::create();
        s_currentPopup = popup;
        Loader::get()->queueInMainThread([popup] {
            popup->show();
        });
    }
};

CaptchaPopup* CaptchaPopup::s_currentPopup = nullptr;

template <typename T, typename Func>
void handleHttpRequest(T* self, Func originalFunc, gd::string url, gd::string params, gd::string tag, GJHttpType type) {
    auto chance = Mod::get()->getSettingValue<int64_t>("chance");
    if (chance > 0 && !CaptchaPopup::isPopupVisible()) {
        auto randVal = std::rand() % 100;
        if (randVal < chance) {
            CaptchaPopup::showPopup();
        }
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
