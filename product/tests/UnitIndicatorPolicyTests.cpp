// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include <entt/entt.hpp>

#include "CommonGameObjectComponents.h"
#include "ECSHelper.h"
#include "ErrorReporter.h"
#include "Player.h"
#include "UnitComponents.h"

// a unit shows its health bar and facing arrow while it is selected or has
// recently been in a fight. that rule reads two pieces of registry state, and
// these cover the half of it that is reachable without a running world: that
// selecting and deselecting a unit actually puts SelectedComponent into the
// registry and takes it out again, since the rule is worthless if the state
// it reads is never written.
//
// the combat half is set in WorldManager's combat pass and aged out in
// RefreshUnitIndicators, neither of which can be reached from outside
// WorldManager - the component's own behaviour is covered below instead

namespace {
// the components ECSHelper::ValidateEntitySelection insists on before it will
// approve a selection. the Ogre::Entity stays null on purpose - approval only
// puts it in a queued material change, which nothing here dispatches
entt::entity MakeSelectableEntity(entt::registry* Registry, Player* Owner,
                                  int PlayerID) {
  const entt::entity Entity = Registry->create();
  Registry->emplace<OwnershipComponent>(Entity, PlayerID, Owner);
  Registry->emplace<MeshComponent>(Entity, nullptr, "mesh", "ent");
  return Entity;
}
}  // namespace

TEST_CASE("Indicator policy - selecting a unit records who selected it") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);
  Player Owner(4);

  const entt::entity Entity = MakeSelectableEntity(&Registry, &Owner, 4);

  Factory.FactoryQueue->Enqueue(
      TrySelectEntityEvent(Entity, &Owner, [](entt::entity) {}));
  Factory.FactoryQueue->Dispatch();

  SelectedComponent* Selected =
      Factory.TryGetComponent<SelectedComponent>(Entity);
  REQUIRE(Selected != nullptr);
  // the player id, not a bare tag - a bar held open by a selection is private
  // to the player who made it
  CHECK(Selected->PlayerID == 4);
}

TEST_CASE("Indicator policy - deselecting a unit takes the record away "
         "again") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);
  Player Owner(1);

  const entt::entity Entity = MakeSelectableEntity(&Registry, &Owner, 1);

  Factory.FactoryQueue->Enqueue(
      TrySelectEntityEvent(Entity, &Owner, [](entt::entity) {}));
  Factory.FactoryQueue->Dispatch();
  REQUIRE(Factory.TryGetComponent<SelectedComponent>(Entity) != nullptr);

  Factory.FactoryQueue->Enqueue(TryUnselectEntityEvent(Entity));
  Factory.FactoryQueue->Dispatch();

  CHECK(Factory.TryGetComponent<SelectedComponent>(Entity) == nullptr);
}

TEST_CASE("Indicator policy - a rejected selection records nothing, so an "
         "enemy unit never shows a bar for the wrong player") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);
  Player Owner(1);
  Player Rival(2);

  const entt::entity Enemy = MakeSelectableEntity(&Registry, &Rival, 2);

  Factory.FactoryQueue->Enqueue(
      TrySelectEntityEvent(Enemy, &Owner, [](entt::entity) {}));
  Factory.FactoryQueue->Dispatch();

  CHECK(Factory.TryGetComponent<SelectedComponent>(Enemy) == nullptr);
}

TEST_CASE("Indicator policy - selecting an already selected unit leaves one "
         "record, not two") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);
  Player Owner(3);

  const entt::entity Entity = MakeSelectableEntity(&Registry, &Owner, 3);

  for (int Attempt = 0; Attempt < 3; ++Attempt) {
    Factory.FactoryQueue->Enqueue(
        TrySelectEntityEvent(Entity, &Owner, [](entt::entity) {}));
  }
  Factory.FactoryQueue->Dispatch();

  SelectedComponent* Selected =
      Factory.TryGetComponent<SelectedComponent>(Entity);
  REQUIRE(Selected != nullptr);
  CHECK(Selected->PlayerID == 3);
}

TEST_CASE("Indicator policy - deselecting a unit that was never selected is "
         "harmless") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);
  Player Owner(1);

  const entt::entity Entity = MakeSelectableEntity(&Registry, &Owner, 1);

  Factory.FactoryQueue->Enqueue(TryUnselectEntityEvent(Entity));
  CHECK_NOTHROW(Factory.FactoryQueue->Dispatch());
  CHECK(Factory.TryGetComponent<SelectedComponent>(Entity) == nullptr);
}

TEST_CASE("Indicator policy - the combat window carries the seconds it was "
         "opened with") {
  entt::registry Registry;
  const entt::entity Entity = Registry.create();

  Registry.emplace<InCombatComponent>(Entity, 3.f);
  REQUIRE(Registry.all_of<InCombatComponent>(Entity));
  CHECK(Registry.get<InCombatComponent>(Entity).Remaining ==
        doctest::Approx(3.f));
}

TEST_CASE("Indicator policy - being hit again restarts the window rather "
         "than stacking a second one") {
  entt::registry Registry;
  const entt::entity Entity = Registry.create();

  // emplace_or_replace is what the combat pass uses, so a unit under
  // sustained attack keeps its indicators up for the full window after the
  // LAST hit rather than the first
  Registry.emplace_or_replace<InCombatComponent>(Entity, 3.f);
  Registry.get<InCombatComponent>(Entity).Remaining = 0.4f;
  Registry.emplace_or_replace<InCombatComponent>(Entity, 3.f);

  CHECK(Registry.get<InCombatComponent>(Entity).Remaining ==
        doctest::Approx(3.f));
}
