#include "RenderSystem.h"

void RenderSystem::RenderLoop() {

}
ViewPortController* RenderSystem::CreateViewPort() {
	return NULL;
}

RenderSystem& RenderSystem::getInstance() {
	static RenderSystem Instance;
	return Instance;
}

