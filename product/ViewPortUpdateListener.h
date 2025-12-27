// Copyright © 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>
#include <OGRE/Overlay/OgreOverlay.h>
#include <unordered_map>
#include "ViewPortController.h"
#include "ErrorReporter.h"

class ViewPortUpdateListener : public Ogre::RenderTargetListener {
private:
    std::unordered_map<Ogre::Overlay*, ViewPortController*> InstanceViewports;
    ErrorReporter VPULReporter;

public:
    ViewPortUpdateListener();

    void preViewportUpdate(const Ogre::RenderTargetViewportEvent& evt) override;

    void AssignOverlayToViewport(ViewPortController* Viewport, Ogre::Overlay* Overlay);
};