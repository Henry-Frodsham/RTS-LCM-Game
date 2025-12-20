// Copyright © 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>
#include <OGRE/Overlay/OgreOverlay.h>
#include <unordered_map>
#include "ViewPortController.h"
class ViewPortUpdateListener : public Ogre::RenderTargetListener {
private:
    std::unordered_map<Ogre::Overlay*, ViewPortController*> InstanceViewports;

public:
    void preViewportUpdate(const Ogre::RenderTargetViewportEvent& evt) override;

    void AssignOverlayToViewport(Ogre::Viewport* vp, Ogre::Overlay* overlay);

    void RegisterInstanceOverlay(Ogre::Overlay* Overlay, ViewPortController* Viewport);
};