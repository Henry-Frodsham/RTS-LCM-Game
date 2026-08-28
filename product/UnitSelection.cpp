// Copyright (c) 2026 Henry Frodsham
#include "UnitSelection.h"

#include <algorithm>
#include <vector>

UnitSelection::UnitSelection(ECSHelper* Interactor, Player* Owner)
    : Factory(Interactor), SelectingPlayer(Owner) {}

bool UnitSelection::Contains(entt::entity Entity) const {
  return std::find(Selected.begin(), Selected.end(), Entity) != Selected.end();
}

entt::entity UnitSelection::Lead() const {
  return Selected.empty() ? entt::null : Selected.front();
}

void UnitSelection::Request(entt::entity Entity, EventQueue* ReplyTo) {
  if (Entity == entt::null || !Factory || !ReplyTo) {
    return;
  }
  if (Contains(Entity)) {
    return;
  }

  Factory->FactoryQueue->Enqueue(
      TrySelectEntityEvent(Entity, SelectingPlayer, [ReplyTo](entt::entity E) {
        ReplyTo->Enqueue(SelectEntitySuccessEvent(E));
      }));
}

void UnitSelection::RequestMany(const std::vector<entt::entity>& Entities,
                                EventQueue* ReplyTo) {
  for (entt::entity Entity : Entities) {
    Request(Entity, ReplyTo);
  }
}

void UnitSelection::Confirm(entt::entity Entity) {
  if (Entity == entt::null || Contains(Entity)) {
    return;
  }
  Selected.push_back(Entity);
}

void UnitSelection::Remove(entt::entity Entity) {
  auto Found = std::find(Selected.begin(), Selected.end(), Entity);
  if (Found == Selected.end()) {
    return;
  }
  Selected.erase(Found);

  if (Factory) {
    Factory->FactoryQueue->Enqueue(TryUnselectEntityEvent(Entity));
  }
}

void UnitSelection::Clear() {
  if (Factory) {
    for (entt::entity Entity : Selected) {
      Factory->FactoryQueue->Enqueue(TryUnselectEntityEvent(Entity));
    }
  }
  Selected.clear();
}

// a destroyed handle is dropped outright rather than unselected - there is
// nothing left to put a material back on, and TryUnselectEntity would only
// find the same invalid handle and give up
void UnitSelection::PruneDestroyed() {
  if (!Factory) {
    return;
  }

  Selected.erase(std::remove_if(Selected.begin(), Selected.end(),
                                [this](entt::entity Entity) {
                                  return !Factory->IsValidEntity(Entity);
                                }),
                 Selected.end());
}
