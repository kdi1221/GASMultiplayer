// KJY All Rights Reserved

#include "AGPGameplayTags.h"

namespace AGPGameplayTags
{
#pragma region [Input Tags]
	UE_DEFINE_GAMEPLAY_TAG(Input, "Input");

	UE_DEFINE_GAMEPLAY_TAG(Input_Native, "Input.Native");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Move, "Input.Native.Move");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Look, "Input.Native.Look");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_IngameMenu, "Input.Native.IngameMenu");

	UE_DEFINE_GAMEPLAY_TAG(Input_Ability, "Input.Ability");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Active, "Input.Ability.Active");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Active_CombatMode, "Input.Ability.Active.CombatMode");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Active_CombatMode_Enter, "Input.Ability.Active.CombatMode.Enter");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Active_CombatMode_Exit, "Input.Ability.Active.CombatMode.Exit");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Active_Attack, "Input.Ability.Active.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Active_Attack_1, "Input.Ability.Active.Attack.1");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Active_Attack_2, "Input.Ability.Active.Attack.2");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Active_Special, "Input.Ability.Active.Special");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Active_Special_1, "Input.Ability.Active.Special.1");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Active_Special_2, "Input.Ability.Active.Special.2");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Active_Dodge, "Input.Ability.Active.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Active_Block, "Input.Ability.Active.Block");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Active_TargetLock, "Input.Ability.Active.TargetLock");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Active_Rage, "Input.Ability.Active.Rage");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Active_ConsumeStone, "Input.Ability.Active.ConsumeStone");

	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Event, "Input.Ability.Event");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Event_TargetLock, "Input.Ability.Event.TargetLock");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Event_TargetLock_SwitchTarget, "Input.Ability.Event.TargetLock.SwitchTarget");

	UE_DEFINE_GAMEPLAY_TAG(Input_Lock, "Input.Lock");
#pragma endregion

#pragma region [Item Tags]
	UE_DEFINE_GAMEPLAY_TAG(Items, "Items");

	UE_DEFINE_GAMEPLAY_TAG(Items_Slots, "Items.Slots");
	UE_DEFINE_GAMEPLAY_TAG(Items_Slots_Inventory, "Items.Slots.Inventory");
	UE_DEFINE_GAMEPLAY_TAG(Items_Slots_Equipment, "Items.Slots.Equipment");
	UE_DEFINE_GAMEPLAY_TAG(Items_Slots_Equipment_Weapons, "Items.Slots.Equipment.Weapons");
	UE_DEFINE_GAMEPLAY_TAG(Items_Slots_Equipment_Weapons_Primary, "Items.Slots.Equipment.Weapons.Primary");
	UE_DEFINE_GAMEPLAY_TAG(Items_Slots_Equipment_Weapons_Primary_1, "Items.Slots.Equipment.Weapons.Primary.1");
	UE_DEFINE_GAMEPLAY_TAG(Items_Slots_Equipment_Weapons_Secondary, "Items.Slots.Equipment.Weapons.Secondary");
	UE_DEFINE_GAMEPLAY_TAG(Items_Slots_Equipment_Weapons_Secondary_1, "Items.Slots.Equipment.Weapons.Secondary.1");
	UE_DEFINE_GAMEPLAY_TAG(Items_Slots_Equipment_Cloths, "Items.Slots.Equipment.Cloths");

	UE_DEFINE_GAMEPLAY_TAG(Items_Types, "Items.Types");
	UE_DEFINE_GAMEPLAY_TAG(Items_Types_None, "Items.Types.None");
	UE_DEFINE_GAMEPLAY_TAG(Items_Types_Weapons, "Items.Types.Weapons");
	UE_DEFINE_GAMEPLAY_TAG(Items_Types_Weapons_Axe, "Items.Types.Weapons.Axe");
	UE_DEFINE_GAMEPLAY_TAG(Items_Types_Weapons_Sword, "Items.Types.Weapons.Sword");
	UE_DEFINE_GAMEPLAY_TAG(Items_Types_Weapons_Club, "Items.Types.Weapons.Club");
#pragma endregion

#pragma region [State]
	UE_DEFINE_GAMEPLAY_TAG(State, "State");

	UE_DEFINE_GAMEPLAY_TAG(State_CombatMode, "State.CombatMode");
	UE_DEFINE_GAMEPLAY_TAG(State_CombatMode_ON, "State.CombatMode.ON");
	UE_DEFINE_GAMEPLAY_TAG(State_CombatMode_OFF, "State.CombatMode.OFF");
	UE_DEFINE_GAMEPLAY_TAG(State_CombatMode_Transition, "State.CombatMode.Transition");
	UE_DEFINE_GAMEPLAY_TAG(State_CombatMode_Transition_Entering, "State.CombatMode.Transition.Entering");
	UE_DEFINE_GAMEPLAY_TAG(State_CombatMode_Transition_Exiting, "State.CombatMode.Transition.Exiting");

	UE_DEFINE_GAMEPLAY_TAG(State_DamageResponse, "State.DamageResponse");
	UE_DEFINE_GAMEPLAY_TAG(State_DamageResponse_CrowdControl, "State.DamageResponse.CrowdControl");
	UE_DEFINE_GAMEPLAY_TAG(State_DamageResponse_CrowdControl_HitReact, "State.DamageResponse.CrowdControl.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(State_DamageResponse_Block, "State.DamageResponse.Block");
	UE_DEFINE_GAMEPLAY_TAG(State_DamageResponse_Block_Success, "State.DamageResponse.Block.Success");
	UE_DEFINE_GAMEPLAY_TAG(State_DamageResponse_Block_ParryReady, "State.DamageResponse.Block.ParryReady");
	UE_DEFINE_GAMEPLAY_TAG(State_DamageResponse_Block_EnableCounter, "State.DamageResponse.Block.EnableCounter");

	UE_DEFINE_GAMEPLAY_TAG(State_Rage, "State.Rage");
	UE_DEFINE_GAMEPLAY_TAG(State_Rage_Enter, "State.Rage.Enter");
	UE_DEFINE_GAMEPLAY_TAG(State_Rage_Activating, "State.Rage.Activating");
	UE_DEFINE_GAMEPLAY_TAG(State_Rage_Exit, "State.Rage.Exit");

	UE_DEFINE_GAMEPLAY_TAG(State_Invincible, "State.Invincible");

	UE_DEFINE_GAMEPLAY_TAG(State_Death, "State.Death");

	UE_DEFINE_GAMEPLAY_TAG(State_SpawnIntro, "State.SpawnIntro");

	UE_DEFINE_GAMEPLAY_TAG(State_ActiveAbility, "State.ActiveAbility");
	UE_DEFINE_GAMEPLAY_TAG(State_ActiveAbility_Attacking, "State.ActiveAbility.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_ActiveAbility_Attacking_Light, "State.ActiveAbility.Attacking.Light");
	UE_DEFINE_GAMEPLAY_TAG(State_ActiveAbility_Attacking_Heavy, "State.ActiveAbility.Attacking.Heavy");
	UE_DEFINE_GAMEPLAY_TAG(State_ActiveAbility_Attacking_Counter, "State.ActiveAbility.Attacking.Counter");
	UE_DEFINE_GAMEPLAY_TAG(State_ActiveAbility_Special, "State.ActiveAbility.Special");
	UE_DEFINE_GAMEPLAY_TAG(State_ActiveAbility_Special_Light, "State.ActiveAbility.Special.Light");
	UE_DEFINE_GAMEPLAY_TAG(State_ActiveAbility_Special_Heavy, "State.ActiveAbility.Special.Heavy");
	UE_DEFINE_GAMEPLAY_TAG(State_ActiveAbility_Dodge, "State.ActiveAbility.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(State_ActiveAbility_Dodge_Roll, "State.ActiveAbility.Dodge.Roll");
	UE_DEFINE_GAMEPLAY_TAG(State_ActiveAbility_Defending, "State.ActiveAbility.Defending");
	UE_DEFINE_GAMEPLAY_TAG(State_ActiveAbility_Defending_Shield, "State.ActiveAbility.Defending.Shield");
	UE_DEFINE_GAMEPLAY_TAG(State_ActiveAbility_Summon, "State.ActiveAbility.Summon");
	UE_DEFINE_GAMEPLAY_TAG(State_ActiveAbility_Summon_SubMonster, "State.ActiveAbility.Summon.SubMonster");

	UE_DEFINE_GAMEPLAY_TAG(State_Ability_Ongoing, "State.Ability.Ongoing");
	UE_DEFINE_GAMEPLAY_TAG(State_Ability_Ongoing_Summon, "State.Ability.Ongoing.Summon");

	UE_DEFINE_GAMEPLAY_TAG(State_Interaction, "State.Interaction");
	UE_DEFINE_GAMEPLAY_TAG(State_Interaction_Consume, "State.Interaction.Consume");
	UE_DEFINE_GAMEPLAY_TAG(State_Interaction_Consume_Stone, "State.Interaction.Consume.Stone");
	UE_DEFINE_GAMEPLAY_TAG(State_Interaction_ObjectActivate, "State.Interaction.ObjectActivate");

	/* 적용 보류.. 인벤토리 시스템 구축전까지 */
	/*UE_DEFINE_GAMEPLAY_TAG(State_Interaction_Item, "State.Interaction.Item");
	UE_DEFINE_GAMEPLAY_TAG(State_Interaction_Item_EquipItem, "State.Interaction.Item.EquipItem");
	UE_DEFINE_GAMEPLAY_TAG(State_Interaction_Item_DisrobeEquipment, "State.Interaction.Item.DisrobeEquipment");*/
#pragma endregion

#pragma region [Ability]
	UE_DEFINE_GAMEPLAY_TAG(Ability, "Ability");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Reactive, "Ability.Reactive");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Reactive_CrowdControl, "Ability.Reactive.CrowdControl");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Reactive_CrowdControl_HitReact, "Ability.Reactive.CrowdControl.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Reactive_Dead, "Ability.Reactive.Dead");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Reactive_Dead_ToDeath, "Ability.Reactive.Dead.ToDeath");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Reactive_Dead_Destroy, "Ability.Reactive.Dead.Destroy");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Reactive_Dead_Rebirth, "Ability.Reactive.Dead.Rebirth");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Reactive_Rage, "Ability.Reactive.Rage");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Reactive_Rage_Exit, "Ability.Reactive.Rage.Exit");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Reactive_SpawnIntro, "Ability.Reactive.SpawnIntro");
	 
	UE_DEFINE_GAMEPLAY_TAG(Ability_StateSwitch, "Ability.StateSwitch");
	UE_DEFINE_GAMEPLAY_TAG(Ability_StateSwitch_CombatMode, "Ability.StateSwitch.CombatMode");
	UE_DEFINE_GAMEPLAY_TAG(Ability_StateSwitch_CombatMode_Enter, "Ability.StateSwitch.CombatMode.Enter");
	UE_DEFINE_GAMEPLAY_TAG(Ability_StateSwitch_CombatMode_Exit, "Ability.StateSwitch.CombatMode.Exit");
	UE_DEFINE_GAMEPLAY_TAG(Ability_StateSwitch_Rage, "Ability.StateSwitch.Rage");
	UE_DEFINE_GAMEPLAY_TAG(Ability_StateSwitch_Rage_Enter, "Ability.StateSwitch.Rage.Enter");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Active, "Ability.Active");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Active_Attack, "Ability.Active.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Active_Attack_Base, "Ability.Active.Attack.Base");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Active_Attack_Base_Light, "Ability.Active.Attack.Base.Light");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Active_Attack_Base_Heavy, "Ability.Active.Attack.Base.Heavy");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Active_Special, "Ability.Active.Special");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Active_Special_Light, "Ability.Active.Special.Light");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Active_Special_Heavy, "Ability.Active.Special.Heavy");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Active_Dodge, "Ability.Active.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Active_Dodge_Roll, "Ability.Active.Dodge.Roll");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Active_Defense, "Ability.Active.Defense");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Active_Defense_Shield, "Ability.Active.Defense.Shield");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Active_Summon, "Ability.Active.Summon");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Active_Summon_SubMonster, "Ability.Active.Summon.SubMonster");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_ChainActive, "Ability.ChainActive");
	UE_DEFINE_GAMEPLAY_TAG(Ability_ChainActive_BlockSuccess, "Ability.ChainActive.BlockSuccess");
	UE_DEFINE_GAMEPLAY_TAG(Ability_ChainActive_CounterAttack, "Ability.ChainActive.CounterAttack");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Interaction, "Ability.Interaction");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Interaction_Consume, "Ability.Interaction.Consume");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Interaction_Consume_Stone, "Ability.Interaction.Consume.Stone");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Interaction_ObjectActivate, "Ability.Interaction.ObjectActivate");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Interaction_Item, "Ability.Interaction.Item");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Interaction_Item_EquipItem, "Ability.Interaction.Item.EquipItem");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Interaction_Item_DisrobeEquipment, "Ability.Interaction.Item.DisrobeEquipment");

	UE_DEFINE_GAMEPLAY_TAG(Ability_PlayerControl, "Ability.PlayerControl");
	UE_DEFINE_GAMEPLAY_TAG(Ability_PlayerControl_TargetLock, "Ability.PlayerControl.TargetLock");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Slot, "Ability.Slot");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Slot_Light, "Ability.Slot.Light");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Slot_Heavy, "Ability.Slot.Heavy");
#pragma endregion

#pragma region [Cooldown]
	UE_DEFINE_GAMEPLAY_TAG(Cooldown, "Cooldown");

	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Special, "Cooldown.Special");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Special_Light, "Cooldown.Special.Light");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Special_Heavy, "Cooldown.Special.Heavy");
#pragma endregion

#pragma region [GameplayEvents]
	UE_DEFINE_GAMEPLAY_TAG(Event, "Event");

	UE_DEFINE_GAMEPLAY_TAG(Event_Animation, "Event.Animation");
	UE_DEFINE_GAMEPLAY_TAG(Event_Animation_ComboInput, "Event.Animation.ComboInput");
	UE_DEFINE_GAMEPLAY_TAG(Event_Animation_ComboInput_Enable, "Event.Animation.ComboInput.Enable");
	UE_DEFINE_GAMEPLAY_TAG(Event_Animation_ComboInput_Disable, "Event.Animation.ComboInput.Disable");

	UE_DEFINE_GAMEPLAY_TAG(Event_CollisionHit, "Event.CollisionHit");
	UE_DEFINE_GAMEPLAY_TAG(Event_CollisionHit_Melee, "Event.CollisionHit.Melee");

	UE_DEFINE_GAMEPLAY_TAG(Event_Character, "Event.Character");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Reaction, "Event.Character.Reaction");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Reaction_HitReact, "Event.Character.Reaction.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Reaction_SuccessBlock, "Event.Character.Reaction.SuccessBlock");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Reaction_Death, "Event.Character.Reaction.Death");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Reaction_Death_Dying, "Event.Character.Reaction.Death.Dying");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Reaction_Death_Death, "Event.Character.Reaction.Death.Death");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Reaction_Death_Destroy, "Event.Character.Reaction.Death.Destroy");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Reaction_Death_Rebirth, "Event.Character.Reaction.Death.Rebirth");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Reaction_Death_None, "Event.Character.Reaction.Death.None");

	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Rage, "Event.Character.Rage");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Rage_ApplyGE, "Event.Character.Rage.ApplyGE");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Rage_Depleted, "Event.Character.Rage.Depleted");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Rage_SpawnSlashProjectile, "Event.Character.Rage.SpawnSlashProjectile");

	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Ability, "Event.Character.Ability");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Ability_AOE, "Event.Character.Ability.AOE");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Ability_Spawn, "Event.Character.Ability.Spawn");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Ability_Spawn_Projectile, "Event.Character.Ability.Spawn.Projectile");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Ability_Spawn_SubMonster, "Event.Character.Ability.Spawn.SubMonster");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Ability_Spawn_Intro, "Event.Character.Ability.Spawn.Intro");

	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Ability_Warning, "Event.Character.Ability.Warning");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Ability_Warning_Unblockable, "Event.Character.Ability.Warning.Unblockable");

	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Ability_Dodge, "Event.Character.Ability.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Ability_Dodge_Invincible, "Event.Character.Ability.Dodge.Invincible");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Ability_Dodge_Invincible_Enter, "Event.Character.Ability.Dodge.Invincible.Enter");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Ability_Dodge_Invincible_Exit, "Event.Character.Ability.Dodge.Invincible.Exit");
#pragma endregion

#pragma region [Animation]
	UE_DEFINE_GAMEPLAY_TAG(Animation, "Animation");

	UE_DEFINE_GAMEPLAY_TAG(Animation_SetProperty, "Animation.SetProperty");
	UE_DEFINE_GAMEPLAY_TAG(Animation_SetProperty_AnimRefreshBone, "Animation.SetProperty.AnimRefreshBone");
#pragma endregion

#pragma region [GameplayEffect]
	UE_DEFINE_GAMEPLAY_TAG(GameplayEffect, "GameplayEffect");

	UE_DEFINE_GAMEPLAY_TAG(GameplayEffect_SetByCaller, "GameplayEffect.SetByCaller");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEffect_SetByCaller_ComboAttackCount, "GameplayEffect.SetByCaller.ComboAttackCount");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEffect_SetByCaller_ComboAttackRate, "GameplayEffect.SetByCaller.ComboAttackRate");
#pragma endregion

#pragma region [GameplayCue]
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_AttackType, "GameplayCue.AttackType");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_AttackType_Sharp, "GameplayCue.AttackType.Sharp");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_AttackType_Sharp_Axe, "GameplayCue.AttackType.Sharp.Axe");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_AttackType_Blunt, "GameplayCue.AttackType.Blunt");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_AttackType_Blunt_Club, "GameplayCue.AttackType.Blunt.Club");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_HitType, "GameplayCue.HitType");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_HitType_Organic, "GameplayCue.HitType.Organic");
#pragma endregion
}