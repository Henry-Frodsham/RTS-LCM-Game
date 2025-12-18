#include "GameInstance.h"

#include "InstanceEvent.h"
GameInstance::GameInstance(ErrorReporter* ParentR, EventQueue* ParentQ,
                           InputDevice* Device,
                           InputTranslator* DeviceTranslator)
    : ParentReporter(ParentR),
      UpstreamQueue(ParentQ),
      InstanceDevice(Device),
      InstanceTranslator(DeviceTranslator) {
  LocalBus = new EventBus();
  LocalQueue = new EventQueue(LocalBus);
}

void GameInstance::Run(float DT) {
  InstanceTranslator->Update(DT);

  if (InstanceTranslator->HasRelativeMotion()) {
    UpstreamQueue->Enqueue(UpstreamOrbitViewport2DEvent(
        InstanceTranslator->GetRelativeMotion(), this));
  }
}
