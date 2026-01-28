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

  LocalBus->Subscribe<ResizedViewPortEvent>(std::bind(&InputTranslator::ResizeViewPortDimensions, InstanceTranslator, std::placeholders::_1));

  InstanceCursor = new Cursor(LocalBus, InstanceTranslator);
}

void GameInstance::Run(float DT) {
  InstanceTranslator->Update(DT);
  InstanceCursor->Update();
  LocalQueue->Dispatch();

  if (InstanceTranslator->HasRelativeMotion()) {
    UpstreamQueue->Enqueue(UpstreamOrbitViewport2DEvent(
        InstanceTranslator->GetRelativeMotion(), this));
  }
}
