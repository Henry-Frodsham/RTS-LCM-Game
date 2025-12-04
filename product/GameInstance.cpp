#include "GameInstance.h"

GameInstance::GameInstance(ErrorReporter* ParentR, EventQueue* ParentQ, InputDevice* Device, InputTranslator* DeviceTranslator)
	: ParentReporter(ParentR)
	, UpstreamQueue(ParentQ)
	, InstanceDevice(Device)
	, InstanceTranslator(DeviceTranslator){
	LocalBus = new EventBus();
	LocalQueue = new EventQueue(LocalBus);
}

void GameInstance::Run(float DT) {
	InstanceTranslator->Update(DT);

}