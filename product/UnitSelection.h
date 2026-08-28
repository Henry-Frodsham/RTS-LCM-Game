// Copyright (c) 2026 Henry Frodsham
#pragma once
#include <entt/entt.hpp>
#include <cstddef>  // NOLINT(build/include_order)
#include <vector>   // NOLINT(build/include_order)

#include "ECSFactoryEvent.h"
#include "ECSHelper.h"
#include "EventQueue.h"
#include "ShareInfoEvent.h"

// the set of units one player currently has selected.
//
// this used to be a single entt::entity member on InteractionWheel, which
// made "the selection" and "the thing the interaction wheel is pointed at"
// the same variable. they are not the same thing once a box drag can pick up
// a dozen units at once, so the set lives here and the wheel asks it
// questions.
//
// membership is not decided here. a handle only joins once ECSHelper has
// confirmed the asking player actually owns it, which is a round trip over
// the factory queue - Request sends the question, Confirm records the answer.
// that is deliberately the same validation a single click already went
// through, so a box drag cannot become a way to select somebody else's army
class UnitSelection {
 public:
  UnitSelection(ECSHelper* Interactor, Player* Owner);

  bool Contains(entt::entity Entity) const;
  bool IsEmpty() const { return Selected.empty(); }
  std::size_t Size() const { return Selected.size(); }

  // the entity that stands in for the group where only one will do - the path
  // preview draws one line, not one per unit. the oldest surviving member, so
  // it doesnt wander between frames while units are added
  entt::entity Lead() const;

  const std::vector<entt::entity>& Entities() const { return Selected; }

  // ask whether Entity may join. answers arrive as a SelectEntitySuccessEvent
  // on ReplyTo, which the caller is expected to route back into Confirm.
  // an entity already in the set is ignored rather than asked about twice
  void Request(entt::entity Entity, EventQueue* ReplyTo);
  void RequestMany(const std::vector<entt::entity>& Entities,
                   EventQueue* ReplyTo);

  // ECSHelper approved a Request. also the point the unit gets its selected
  // material, so nothing is ever highlighted that isnt in the set
  void Confirm(entt::entity Entity);

  // shift-clicking a unit thats already selected drops it. no round trip -
  // it had to be validated to get in, and taking something away can never
  // grant the player anything they didnt already have
  void Remove(entt::entity Entity);

  // put every member back to its unselected material and empty the set
  void Clear();

  // empty the set without unselecting anything. for the case where the
  // entities are on their way out anyway - there is no material left to
  // restore on something that is being destroyed
  void Reset() { Selected.clear(); }

  // drop handles the registry has since destroyed. combat kills units out
  // from under a selection, and a stale handle is not safe to ask about
  // components, so this runs before anything walks the set
  void PruneDestroyed();

 private:
  ECSHelper* Factory;
  Player* SelectingPlayer;

  // insertion ordered rather than a set - Lead() means "the first one still
  // here", and a selection is a handful of units, not a collection worth
  // hashing
  std::vector<entt::entity> Selected;
};
