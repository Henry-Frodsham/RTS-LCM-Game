#include "GameInstance.h"

#include "InstanceEvent.h"


GameInstance::GameInstance(ErrorReporter* ParentR, EventQueue* ParentQ,
                           InputDevice* Device,
                           InputTranslator* DeviceTranslator, int ThreadNumber)
    : ParentReporter(ParentR),
      UpstreamQueue(ParentQ),
      InstanceDevice(Device),
      InstanceTranslator(DeviceTranslator),
      InstanceNumber(ThreadNumber){
  LocalBus = new EventBus();
  LocalQueue = new EventQueue(LocalBus);

  LocalBus->Subscribe<ResizedViewPortEvent>(std::bind(&InputTranslator::ResizeViewPortDimensions, InstanceTranslator, std::placeholders::_1));

  InstanceCursor = new Cursor(LocalBus, InstanceTranslator, InstanceNumber);
}

void GameInstance::Run(float DT) {
  InstanceTranslator->Update(DT);
  

  if (InstanceTranslator->HasRelativeMotion()) {
    UpstreamQueue->Enqueue(UpstreamOrbitViewport2DEvent(
        InstanceTranslator->GetRelativeMotion(), this));
    InstanceCursor->Update();
  }
  LocalQueue->Dispatch();
}
