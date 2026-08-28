// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include <entt/entt.hpp>
#include <vector>

#include "ECSHelper.h"
#include "ErrorReporter.h"
#include "EventBus.h"
#include "EventQueue.h"
#include "Player.h"
#include "UnitSelection.h"

namespace {
// the components ECSHelper::ValidateEntitySelection insists on before it will
// approve a selection. emplaced straight onto the registry rather than built
// through the factory events, because the real construction path wants a live
// ogre scene node and none of what is being tested here does. the Ogre::Entity
// stays null on purpose - validation only ever puts it in a queued material
// change, which nothing in these tests dispatches
entt::entity MakeSelectableEntity(entt::registry* Registry, Player* Owner) {
  const entt::entity Entity = Registry->create();
  Registry->emplace<OwnershipComponent>(Entity, 1, Owner);
  Registry->emplace<MeshComponent>(Entity, nullptr, "mesh", "ent");
  return Entity;
}

// routes the factory's approval back into the selection, the way
// InteractionWheel does with its ForeignNotifQueue
struct ReplyChannel {
  EventBus Bus;
  EventQueue Queue{&Bus};

  explicit ReplyChannel(UnitSelection* Selection) {
    Bus.Subscribe<SelectEntitySuccessEvent>(
        [Selection](const SelectEntitySuccessEvent& Event) {
          Selection->Confirm(Event.Entity);
        });
  }
};
}  // namespace

TEST_CASE("UnitSelection - a fresh selection is empty and has no lead") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);
  Player Owner(1);

  UnitSelection Selection(&Factory, &Owner);

  CHECK(Selection.IsEmpty());
  CHECK(Selection.Size() == 0);
  // compared into a bool first - doctest's expression decomposition cannot
  // pick between entt::null's comparison operators on its own
  const bool HasNoLead = (Selection.Lead() == entt::null);
  CHECK(HasNoLead);
  CHECK_FALSE(Selection.Contains(entt::null));
}

TEST_CASE("UnitSelection - Confirm adds an entity and the first one leads") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);
  Player Owner(1);

  UnitSelection Selection(&Factory, &Owner);
  const entt::entity First = Registry.create();
  const entt::entity Second = Registry.create();

  Selection.Confirm(First);
  Selection.Confirm(Second);

  CHECK(Selection.Size() == 2);
  CHECK(Selection.Contains(First));
  CHECK(Selection.Contains(Second));
  CHECK(Selection.Lead() == First);
}

TEST_CASE("UnitSelection - Confirming the same entity twice does not "
         "duplicate it") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);
  Player Owner(1);

  UnitSelection Selection(&Factory, &Owner);
  const entt::entity Entity = Registry.create();

  Selection.Confirm(Entity);
  Selection.Confirm(Entity);

  CHECK(Selection.Size() == 1);
}

TEST_CASE("UnitSelection - Confirm ignores a null handle") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);
  Player Owner(1);

  UnitSelection Selection(&Factory, &Owner);
  Selection.Confirm(entt::null);

  CHECK(Selection.IsEmpty());
}

TEST_CASE("UnitSelection - Remove drops one member and the lead moves on") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);
  Player Owner(1);

  UnitSelection Selection(&Factory, &Owner);
  const entt::entity First = Registry.create();
  const entt::entity Second = Registry.create();

  Selection.Confirm(First);
  Selection.Confirm(Second);
  Selection.Remove(First);

  CHECK(Selection.Size() == 1);
  CHECK_FALSE(Selection.Contains(First));
  CHECK(Selection.Lead() == Second);
}

TEST_CASE("UnitSelection - Removing something that was never selected changes "
         "nothing") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);
  Player Owner(1);

  UnitSelection Selection(&Factory, &Owner);
  const entt::entity Selected = Registry.create();
  const entt::entity Stranger = Registry.create();

  Selection.Confirm(Selected);
  Selection.Remove(Stranger);

  CHECK(Selection.Size() == 1);
  CHECK(Selection.Contains(Selected));
}

TEST_CASE("UnitSelection - Clear and Reset both empty the set") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);
  Player Owner(1);

  UnitSelection Selection(&Factory, &Owner);
  Selection.Confirm(Registry.create());
  Selection.Confirm(Registry.create());
  Selection.Clear();
  CHECK(Selection.IsEmpty());

  Selection.Confirm(Registry.create());
  Selection.Reset();
  CHECK(Selection.IsEmpty());
}

TEST_CASE("UnitSelection - PruneDestroyed drops handles the registry has "
         "thrown away and keeps the rest") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);
  Player Owner(1);

  UnitSelection Selection(&Factory, &Owner);
  const entt::entity Survivor = Registry.create();
  const entt::entity Casualty = Registry.create();

  Selection.Confirm(Survivor);
  Selection.Confirm(Casualty);

  Registry.destroy(Casualty);
  Selection.PruneDestroyed();

  CHECK(Selection.Size() == 1);
  CHECK(Selection.Contains(Survivor));
  CHECK(Selection.Lead() == Survivor);
}

TEST_CASE("UnitSelection - Request only adds once the factory has approved "
         "the entity") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);
  Player Owner(1);

  UnitSelection Selection(&Factory, &Owner);
  ReplyChannel Reply(&Selection);

  const entt::entity Entity = MakeSelectableEntity(&Registry, &Owner);

  Selection.Request(Entity, &Reply.Queue);
  // the question is still on the factory queue, nothing has answered it
  CHECK(Selection.IsEmpty());

  Factory.FactoryQueue->Dispatch();
  Reply.Queue.Dispatch();

  CHECK(Selection.Size() == 1);
  CHECK(Selection.Contains(Entity));
}

TEST_CASE("UnitSelection - Request never adds an entity another player owns") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);
  Player Owner(1);
  Player Rival(2);

  UnitSelection Selection(&Factory, &Owner);
  ReplyChannel Reply(&Selection);

  const entt::entity Enemy = MakeSelectableEntity(&Registry, &Rival);

  Selection.Request(Enemy, &Reply.Queue);
  Factory.FactoryQueue->Dispatch();
  Reply.Queue.Dispatch();

  CHECK(Selection.IsEmpty());
}

TEST_CASE("UnitSelection - RequestMany selects every owned entity in the "
         "list and skips the rest") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);
  Player Owner(1);
  Player Rival(2);

  UnitSelection Selection(&Factory, &Owner);
  ReplyChannel Reply(&Selection);

  const entt::entity Mine = MakeSelectableEntity(&Registry, &Owner);
  const entt::entity AlsoMine = MakeSelectableEntity(&Registry, &Owner);
  const entt::entity Theirs = MakeSelectableEntity(&Registry, &Rival);

  Selection.RequestMany({Mine, Theirs, AlsoMine}, &Reply.Queue);
  Factory.FactoryQueue->Dispatch();
  Reply.Queue.Dispatch();

  CHECK(Selection.Size() == 2);
  CHECK(Selection.Contains(Mine));
  CHECK(Selection.Contains(AlsoMine));
  CHECK_FALSE(Selection.Contains(Theirs));
}

TEST_CASE("UnitSelection - Request ignores a null handle and a null reply "
         "queue") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);
  Player Owner(1);

  UnitSelection Selection(&Factory, &Owner);
  ReplyChannel Reply(&Selection);

  const entt::entity Entity = MakeSelectableEntity(&Registry, &Owner);

  Selection.Request(entt::null, &Reply.Queue);
  Selection.Request(Entity, nullptr);
  Factory.FactoryQueue->Dispatch();
  Reply.Queue.Dispatch();

  CHECK(Selection.IsEmpty());
}
