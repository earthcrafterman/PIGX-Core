/*
 * processor.cpp
 *
 *  Created on: 2010-8-3
 *      Author: Argon
 */

#include <iterator>
#include <algorithm>
#include "field.h"
#include "duel.h"
#include "card.h"
#include "group.h"
#include "effect.h"
#include "interpreter.h"

void field::add_process(uint16_t type, uint16_t step, effect* peffect, group* target, int64_t arg1, int64_t arg2, int64_t arg3, int64_t arg4, void* ptr1, void* ptr2) {
	core.subunits.emplace_back();
	processor_unit& new_unit = core.subunits.back();
	new_unit.type = type;
	new_unit.step = step;
	new_unit.peffect = peffect;
	new_unit.ptarget = target;
	new_unit.arg1 = arg1;
	new_unit.arg2 = arg2;
	new_unit.arg3 = arg3;
	new_unit.arg4 = arg4;
	new_unit.ptr1 = ptr1;
	new_unit.ptr2 = ptr2;
}
int32_t field::process() {
	if (core.subunits.size())
		core.units.splice(core.units.begin(), core.subunits);
	if (core.units.size() == 0)
		return PROCESSOR_FLAG_END;
	auto it = core.units.begin();
	switch (it->type) {
	case PROCESSOR_ADJUST: {
		if (adjust_step(it->step))
			core.units.pop_front();
		else {
			++it->step;
		}
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_TURN: {
		if (process_turn(it->step, it->arg1))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_REFRESH_LOC: {
		if (refresh_location_info(it->step))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_STARTUP: {
		if (startup(it->step))
			core.units.pop_front();
		else {
			++it->step;
		}
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_SELECT_BATTLECMD: {
		if (select_battle_command(it->step, it->arg1)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			it->step = 1;
			return PROCESSOR_FLAG_WAITING;
		}
	}
	case PROCESSOR_SELECT_IDLECMD: {
		if (select_idle_command(it->step, it->arg1)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			it->step = 1;
			return PROCESSOR_FLAG_WAITING;
		}
	}
	case PROCESSOR_SELECT_EFFECTYN: {
		if (select_effect_yes_no(it->step, it->arg1, it->arg2, (card*)it->ptarget)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			it->step = 1;
			return PROCESSOR_FLAG_WAITING;
		}
	}
	case PROCESSOR_SELECT_YESNO: {
		if (select_yes_no(it->step, it->arg1, it->arg2)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			it->step = 1;
			return PROCESSOR_FLAG_WAITING;
		}
	}
	case PROCESSOR_SELECT_OPTION: {
		if (select_option(it->step, it->arg1)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			it->step = 1;
			return PROCESSOR_FLAG_WAITING;
		}
	}
	case PROCESSOR_SELECT_CARD: {
		if (select_card(it->step, it->arg1 & 0xff, (it->arg1 >> 16) & 0xff, (it->arg2) & 0xff, (it->arg2 >> 16) & 0xff)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			it->step = 1;
			return PROCESSOR_FLAG_WAITING;
		}
	}
	case PROCESSOR_SELECT_CARD_CODES: {
		if (select_card_codes(it->step, it->arg1 & 0xff, (it->arg1 >> 16) & 0xff, (it->arg2) & 0xff, (it->arg2 >> 16) & 0xff)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			it->step = 1;
			return PROCESSOR_FLAG_WAITING;
		}
	}
	case PROCESSOR_SELECT_UNSELECT_CARD: {
		if (select_unselect_card(it->step, it->arg1 & 0xff, (it->arg1 >> 16) & 0xff, (it->arg2) & 0xff, (it->arg2 >> 16) & 0xff, (it->arg3) & 0xff)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			it->step = 1;
			return PROCESSOR_FLAG_WAITING;
		}
	}
	case PROCESSOR_SELECT_CHAIN: {
		if (select_chain(it->step, it->arg1, (it->arg2 & 0xffff), it->arg2 >> 16)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			it->step = 1;
			return PROCESSOR_FLAG_WAITING;
		}
	}
	case PROCESSOR_SELECT_DISFIELD:
	case PROCESSOR_SELECT_PLACE: {
		if(select_place(it->step, it->arg1, it->arg2, it->arg3)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			it->step = 1;
			return PROCESSOR_FLAG_WAITING;
		}
	}
	case PROCESSOR_SELECT_POSITION: {
		if (select_position(it->step, it->arg1 & 0xffff, it->arg2, (it->arg1 >> 16) & 0xffff)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			it->step = 1;
			return PROCESSOR_FLAG_WAITING;
		}
	}
	case PROCESSOR_SELECT_TRIBUTE_P: {
		if (select_tribute(it->step, it->arg1 & 0xff, (it->arg1 >> 16) & 0xff, (it->arg2) & 0xff, (it->arg2 >> 16) & 0xff)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			it->step = 1;
			return PROCESSOR_FLAG_WAITING;
		}
	}
	case PROCESSOR_SORT_CHAIN: {
		if(sort_chain(it->step, it->arg1)) {
			core.units.pop_front();
		} else {
			++it->step;
		}
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_SELECT_COUNTER: {
		if (select_counter(it->step, it->arg1, it->arg2, it->arg3, it->arg4 >> 8, it->arg4 & 0xff)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			it->step = 1;
			return PROCESSOR_FLAG_WAITING;
		}
	}
	case PROCESSOR_SELECT_SUM: {
		if (select_with_sum_limit(it->step, it->arg2 & 0xffff, it->arg1, (it->arg2 >> 16) & 0xff, (it->arg2 >> 24) & 0xff)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			it->step = 1;
			return PROCESSOR_FLAG_WAITING;
		}
	}
	case PROCESSOR_SORT_CARD: {
		if (sort_card(it->step, it->arg1, it->arg2)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			it->step = 1;
			return PROCESSOR_FLAG_WAITING;
		}
	}
	case PROCESSOR_SELECT_RELEASE: {
		if (select_release_cards(it->step, it->arg1 & 0xff, (it->arg1 >> 16) & 0xff, (it->arg2) & 0xff, (it->arg2 >> 16) & 0xff, it->arg3 & 0xff, (card*)it->ptarget, (it->arg3 >> 8) & 0xff, (it->arg3 >> 16) & 0xff))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_SELECT_TRIBUTE: {
		if (select_tribute_cards(it->step, (card*)it->ptarget, it->arg1 & 0xff, (it->arg1 >> 16) & 0xff, (it->arg2) & 0xff, (it->arg2 >> 16) & 0xff, it->arg3, it->arg4))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_POINT_EVENT: {
		if(process_point_event(it->step, it->arg1 & 0xff, (it->arg1 >> 8) & 0xff, it->arg2))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_QUICK_EFFECT: {
		if(process_quick_effect(it->step, it->arg1, it->arg2))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_IDLE_COMMAND: {
		if(process_idle_command(it->step))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_PHASE_EVENT: {
		if(process_phase_event(it->step, it->arg1))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_BATTLE_COMMAND: {
		if(process_battle_command(it->step))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_DAMAGE_STEP: {
		if(process_damage_step(it->step, it->arg2))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_FORCED_BATTLE: {
		if(process_forced_battle(it->step))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_ADD_CHAIN: {
		if (add_chain(it->step))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_SOLVE_CHAIN: {
		if (solve_chain(it->step, it->arg1, it->arg2))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_SOLVE_CONTINUOUS: {
		if (solve_continuous(it->step))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_EXECUTE_COST: {
		if (execute_cost(it->step, it->peffect, it->arg1)) {
			core.units.pop_front();
			core.solving_event.pop_front();
		} else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_EXECUTE_OPERATION: {
		if (execute_operation(it->step, it->peffect, it->arg1)) {
			core.units.pop_front();
			core.solving_event.pop_front();
		} else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_EXECUTE_TARGET: {
		if (execute_target(it->step, it->peffect, it->arg1)) {
			core.units.pop_front();
			core.solving_event.pop_front();
		} else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_DESTROY: {
		if (destroy(it->step, it->ptarget, it->peffect, it->arg1, it->arg2))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_RELEASE: {
		if (release(it->step, it->ptarget, it->peffect, it->arg1, it->arg2))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_SENDTO: {
		if (send_to(it->step, it->ptarget, it->peffect, it->arg1, it->arg2))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_DESTROY_REPLACE: {
		if(destroy_replace(it->step, it->ptarget, (card*)it->ptr1, it->arg2))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_RELEASE_REPLACE: {
		if (release_replace(it->step, it->ptarget, (card*)it->ptr1))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_SENDTO_REPLACE: {
		if (send_replace(it->step, it->ptarget, (card*)it->ptr1))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_MOVETOFIELD: {
		if (move_to_field(it->step, (card*)it->ptarget, it->arg1 & 0xff, (it->arg1 >> 8) & 0xff, (it->arg1 >> 16) & 0xff, (it->arg1 >> 24) & 0xff, it->arg2 & 0xff, (it->arg2 >> 8) & 0xff, (it->arg2 >> 16) & 0xff))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_CHANGEPOS: {
		if (change_position(it->step, it->ptarget, it->peffect, it->arg1, it->arg2))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_OPERATION_REPLACE: {
		if (operation_replace(it->step, it->peffect, it->ptarget, (card*)it->ptr1, it->arg1))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_ACTIVATE_EFFECT: {
		if (activate_effect(it->step, it->peffect))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_SUMMON_RULE: {
		if (summon(it->step, it->arg1 & 0xff, (card*)it->ptarget, it->peffect, (it->arg1 >> 8) & 0xff, (it->arg1 >> 16) & 0xff, (it->arg1 >> 24) & 0xff))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_SPSUMMON_RULE: {
		if (special_summon_rule(it->step, it->arg1, (card*)it->ptarget, it->arg2))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_SPSUMMON: {
		if (special_summon(it->step, it->peffect, it->arg1, it->ptarget, it->arg2))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_FLIP_SUMMON: {
		if (flip_summon(it->step, it->arg1, (card*)(it->ptarget)))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_MSET: {
		if (mset(it->step, it->arg1 & 0xff, (card*)it->ptarget, it->peffect, (it->arg1 >> 8) & 0xff, (it->arg1 >> 16) & 0xff, (it->arg1 >> 24) & 0xff))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_SSET: {
		if (sset(it->step, it->arg1, it->arg2, (card*)(it->ptarget), it->peffect))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_SPSUMMON_STEP: {
		if (special_summon_step(it->step, it->ptarget, (card*)(it->ptr1), it->arg1))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_SSET_G: {
		if (sset_g(it->step, it->arg1, it->arg2, it->ptarget, it->arg3, it->peffect)) {
			core.units.pop_front();
		} else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_SPSUMMON_RULE_G: {
		if (special_summon_rule_group(it->step, it->arg1, it->arg2))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_DRAW	: {
		if (draw(it->step, it->peffect, it->arg1, (it->arg2 >> 28) & 0xf, (it->arg2 >> 24) & 0xf, it->arg2 & 0xffffff))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_DAMAGE: {
		uint32_t reason = static_cast<uint32_t>(it->arg1);
		effect* reason_effect = nullptr;
		card* reason_card = nullptr;
		if(reason & REASON_BATTLE)
			reason_card = static_cast<card*>(it->ptr1);
		else
			reason_effect = it->peffect;
		uint32_t amount = static_cast<uint32_t>(it->arg3);
		uint8_t reason_player = (it->arg2 >> 26) & 0x3;
		uint8_t playerid = (it->arg2 >> 24) & 0x3;
		bool is_step = (it->arg2 >> 28) & 0x1;
		if (damage(it->step, reason_effect, reason, reason_player, reason_card, playerid, amount, is_step)) {
			if(it->step == 9) {
				it->step = 1;
				core.recover_damage_reserve.splice(core.recover_damage_reserve.end(), core.units, it);
			} else
				core.units.pop_front();
		} else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_RECOVER: {
		uint32_t reason = static_cast<uint32_t>(it->arg1);
		uint8_t reason_player = (it->arg2 >> 26) & 0x3;
		uint8_t playerid = (it->arg2 >> 24) & 0x3;
		uint32_t amount = static_cast<uint32_t>(it->arg3);
		bool is_step = (it->arg2 >> 28) & 0x1;
		if (recover(it->step, it->peffect, reason, reason_player, playerid, amount, is_step)) {
			if(it->step == 9) {
				it->step = 1;
				core.recover_damage_reserve.splice(core.recover_damage_reserve.end(), core.units, it);
			} else
				core.units.pop_front();
		} else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_EQUIP: {
		if (equip(it->step, it->arg2 & 0xffff, (card*)it->ptr1, (card*)it->ptarget, (it->arg2 >> 16) & 0xff, (it->arg2 >> 24) & 0xff))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_GET_CONTROL: {
		if (get_control(it->step, it->peffect, (it->arg2 >> 28) & 0xf, it->ptarget, (it->arg2 >> 24) & 0xf, (it->arg2 >> 8) & 0x3ff, it->arg2 & 0xff, it->arg3)) {
			core.units.pop_front();
		} else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_SWAP_CONTROL: {
		if (swap_control(it->step, it->peffect, it->arg1, it->ptarget, (group*)it->ptr1, it->arg2, it->arg3)) {
			core.units.pop_front();
		} else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_CONTROL_ADJUST: {
		if (control_adjust(it->step)) {
			core.units.pop_front();
		} else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_SELF_DESTROY: {
		if (self_destroy(it->step, (card*)it->ptr1, it->arg1)) {
			core.units.pop_front();
		} else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_TRAP_MONSTER_ADJUST: {
		if (trap_monster_adjust(it->step))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_PAY_LPCOST: {
		if (pay_lp_cost(it->step, it->arg1, it->arg2))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_REMOVE_COUNTER: {
		if (remove_counter(it->step, it->arg4, (card*)it->ptarget, (it->arg1 >> 16) & 0xff, (it->arg1 >> 8) & 0xff, it->arg1 & 0xff, it->arg2, it->arg3)) {
			core.units.pop_front();
		} else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_ATTACK_DISABLE: {
		if(it->step == 0) {
			card* attacker = core.attacker;
			if(!attacker
			        || core.effect_damage_step != 0
			        || (attacker->fieldid_r != core.pre_field[0])
			        || (attacker->current.location != LOCATION_MZONE)
			        || !attacker->is_capable_attack()
			        || !attacker->is_affect_by_effect(core.reason_effect)
					|| attacker->is_affected_by_effect(EFFECT_UNSTOPPABLE_ATTACK)) {
				returns.set<int32_t>(0, 0);
				core.units.pop_front();
			} else {
				effect* peffect = pduel->new_effect();
				peffect->code = EFFECT_ATTACK_DISABLED;
				peffect->type = EFFECT_TYPE_SINGLE;
				attacker->add_effect(peffect);
				attacker->set_status(STATUS_ATTACK_CANCELED, TRUE);
				raise_event(attacker, EVENT_ATTACK_DISABLED, core.reason_effect, 0, core.reason_player, PLAYER_NONE, 0);
				process_instant_event();
				++it->step;
			}
		} else {
			returns.set<int32_t>(0, 1);
			core.units.pop_front();
		}
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_ANNOUNCE_RACE: {
		if(announce_race(it->step, it->arg1 & 0xffff, it->arg1 >> 16, it->arg2)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			++it->step;
		}
		return PROCESSOR_FLAG_WAITING;
	}
	case PROCESSOR_ANNOUNCE_ATTRIB: {
		if(announce_attribute(it->step, it->arg1 & 0xffff, it->arg1 >> 16, it->arg2)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			++it->step;
		}
		return PROCESSOR_FLAG_WAITING;
	}
	case PROCESSOR_ANNOUNCE_CARD: {
		if(announce_card(it->step, it->arg1)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			if(it->step == 0)
				++it->step;
		}
		return PROCESSOR_FLAG_WAITING;
	}
	case PROCESSOR_ANNOUNCE_NUMBER: {
		if(announce_number(it->step, it->arg1)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			++it->step;
		}
		return PROCESSOR_FLAG_WAITING;
	}
	case PROCESSOR_TOSS_DICE: {
		if(toss_dice(it->step, it->peffect, it->arg1 >> 16, it->arg1 & 0xff, it->arg2 & 0xff, it->arg2 >> 16)) {
			core.units.pop_front();
		} else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_TOSS_COIN: {
		if (toss_coin(it->step, it->peffect, (it->arg1 >> 16), it->arg1 & 0xff, it->arg2)) {
			core.units.pop_front();
		} else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_ROCK_PAPER_SCISSORS: {
		if (rock_paper_scissors(it->step, it->arg1)) {
			core.units.pop_front();
			return PROCESSOR_FLAG_CONTINUE;
		} else {
			++it->step;
			return PROCESSOR_FLAG_WAITING;
		}
	}
	case PROCESSOR_SELECT_FUSION: {
		if(it->step == 0) {
			effect_set eset;
			card* pcard = (card*)it->ptr2;
			pcard->fusion_filter_valid(it->ptarget, (group*)it->ptr1, it->arg1>>16, &eset);
			core.select_effects.clear();
			core.select_options.clear();
			if(eset.size() < 1) {
				core.units.pop_front();
				return PROCESSOR_FLAG_CONTINUE;
			}
			for(const auto& peff : eset) {
				core.select_effects.push_back(peff);
				core.select_options.push_back(peff->description);
			}
			if (core.select_options.size() == 1)
				returns.set<int32_t>(0, 0);
			else
				add_process(PROCESSOR_SELECT_OPTION, 0, 0, 0, it->arg1, 0);
			++it->step;
		} else if(it->step==1) {
			core.fusion_materials.clear();
			if(!core.select_effects[returns.at<int32_t>(0)]) {
				core.units.pop_front();
				return PROCESSOR_FLAG_CONTINUE;
			}
			core.sub_solving_event.emplace_back();
			auto& e = core.sub_solving_event.back();
			e.event_cards = it->ptarget;
			e.reason_effect = core.select_effects[returns.at<int32_t>(0)];
			e.reason_player = it->arg1;
			pduel->lua->add_param<PARAM_TYPE_GROUP>(it->ptr1);
			pduel->lua->add_param<PARAM_TYPE_INT>(it->arg1 >> 16);
			add_process(PROCESSOR_EXECUTE_OPERATION, 0, core.select_effects[returns.at<int32_t>(0)], 0, it->arg1 & 0xffff, 0);
			++it->step;
		} else {
			core.units.pop_front();
		}
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_DISCARD_HAND: {
		if(it->step == 0) {
			auto message = pduel->new_message(MSG_HINT);
			message->write<uint8_t>(HINT_SELECTMSG);
			message->write<uint8_t>(it->arg1);
			if(it->arg3 & REASON_DISCARD)
				message->write<uint64_t>(501);
			else
				message->write<uint64_t>(504);
			add_process(PROCESSOR_SELECT_CARD, 0, 0, 0, it->arg1, it->arg2);
			++it->step;
		} else if(it->step == 1) {
			if(return_cards.list.empty())
				returns.set<int32_t>(0, 0);
			else
				send_to(card_set{ return_cards.list.begin(), return_cards.list.end() }, core.reason_effect, it->arg3, core.reason_player, PLAYER_NONE, LOCATION_GRAVE, 0, POS_FACEUP);
			++it->step;
		} else {
			core.units.pop_front();
		}
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_DISCARD_DECK: {
		if(discard_deck(it->step, it->arg1 & 0xff, it->arg1 >> 16, it->arg2)) {
			core.units.pop_front();
		} else {
			++it->step;
		}
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_SORT_DECK: {
		uint8_t sort_player = it->arg1 & 0xffff;
		uint8_t target_player = it->arg1 >> 16;
		uint32_t count = static_cast<uint32_t>(it->arg2);
		bool bottom = it->arg3;
		auto& list = player[target_player].list_main;
		if(count > list.size())
			count = static_cast<uint32_t>(list.size());
		if(it->step == 0) {
			if(bottom) {
				const auto clit = list.begin();
				core.select_cards.assign(clit, clit + count);
			} else {
				const auto clit = list.rbegin();
				core.select_cards.assign(clit, clit + count);
			}
			add_process(PROCESSOR_SORT_CARD, 0, 0, 0, sort_player, 0);
			++it->step;
		} else {
			if(returns.at<int8_t>(0) != -1 && count > 0) {
				card* tc[64];
				for(uint32_t i = 0; i < count; ++i)
					tc[returns.at<uint8_t>(i)] = core.select_cards[i];
				for(uint32_t i = 0; i < count; ++i) {
					card* pcard = nullptr;
					if(bottom)
						pcard = tc[i];
					else
						pcard = tc[count - i - 1];
					auto message = pduel->new_message(MSG_MOVE);
					message->write<uint32_t>(0);
					message->write(pcard->get_info_location());
					list.erase(list.begin() + pcard->current.sequence);
					if(bottom)
						list.insert(list.begin(), pcard);
					else
						list.push_back(pcard);
					reset_sequence(target_player, LOCATION_DECK);
					message->write(pcard->get_info_location());
					message->write<uint32_t>(pcard->current.reason);
				}
				if(core.global_flag & GLOBALFLAG_DECK_REVERSE_CHECK) {
					card* ptop = list.back();
					if(core.deck_reversed || (ptop->current.position == POS_FACEUP_DEFENSE)) {
						auto message = pduel->new_message(MSG_DECK_TOP);
						message->write<uint8_t>(target_player);
						message->write<uint32_t>(0);
						message->write<uint32_t>(ptop->data.code);
						message->write<uint32_t>(ptop->current.position);
					}
				}
			}
			core.units.pop_front();
		}
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_REMOVE_OVERLAY: {
		if(remove_overlay_card(it->step, it->arg3, it->ptarget, it->arg1 >> 16,
		                       (it->arg1 >> 8) & 0xff, it->arg1 & 0xff, it->arg2 & 0xffff, it->arg2 >> 16)) {
			core.units.pop_front();
		} else {
			++it->step;
		}
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_XYZ_OVERLAY: {
		if(xyz_overlay(it->step, static_cast<card*>(it->ptr1), it->ptarget, !!it->arg1)) {
			core.units.pop_front();
		} else {
			++it->step;
		}
		return PROCESSOR_FLAG_CONTINUE;
	}
	case PROCESSOR_REFRESH_RELAY: {
		if (refresh_relay(it->step))
			core.units.pop_front();
		else
			++it->step;
		return PROCESSOR_FLAG_CONTINUE;
	}
	}
	return PROCESSOR_FLAG_CONTINUE;
}
int32_t field::execute_cost(uint16_t step, effect* triggering_effect, uint8_t triggering_player) {
	if(!triggering_effect->cost) {
		core.solving_event.splice(core.solving_event.begin(), core.sub_solving_event);
		pduel->lua->params.clear();
		return TRUE;
	}
	if (step == 0) {
		core.solving_event.splice(core.solving_event.begin(), core.sub_solving_event);
		const tevent& e = core.solving_event.front();
		pduel->lua->add_param<PARAM_TYPE_INT>(1, true);
		pduel->lua->add_param<PARAM_TYPE_INT>(e.reason_player, true);
		pduel->lua->add_param<PARAM_TYPE_INT>(e.reason, true);
		pduel->lua->add_param<PARAM_TYPE_EFFECT>(e.reason_effect , true);
		pduel->lua->add_param<PARAM_TYPE_INT>(e.event_value, true);
		pduel->lua->add_param<PARAM_TYPE_INT>(e.event_player, true);
		pduel->lua->add_param<PARAM_TYPE_GROUP>(e.event_cards , true);
		pduel->lua->add_param<PARAM_TYPE_INT>(triggering_player, true);
		pduel->lua->add_param<PARAM_TYPE_EFFECT>(triggering_effect, true);
		if(core.check_level == 0) {
			core.shuffle_deck_check[0] = FALSE;
			core.shuffle_deck_check[1] = FALSE;
			core.shuffle_hand_check[0] = FALSE;
			core.shuffle_hand_check[1] = FALSE;
		}
		core.units.begin()->arg2 = core.shuffle_check_disabled;
		core.shuffle_check_disabled = FALSE;
		++core.check_level;
	}
	core.reason_effect = triggering_effect;
	core.reason_player = triggering_player;
	uint32_t count = static_cast<uint32_t>(pduel->lua->params.size());
	lua_Integer yield_value = 0;
	int32_t result = pduel->lua->call_coroutine(triggering_effect->cost, count, &yield_value, step);
	returns.set<int32_t>(0, static_cast<int32_t>(yield_value));
	if (result != COROUTINE_YIELD) {
		core.reason_effect = 0;
		core.reason_player = PLAYER_NONE;
		--core.check_level;
		if(core.check_level == 0) {
			if(core.shuffle_hand_check[0])
				shuffle(0, LOCATION_HAND);
			if(core.shuffle_hand_check[1])
				shuffle(1, LOCATION_HAND);
			if(core.shuffle_deck_check[0])
				shuffle(0, LOCATION_DECK);
			if(core.shuffle_deck_check[1])
				shuffle(1, LOCATION_DECK);
		}
		core.shuffle_check_disabled = core.units.begin()->arg2;
		return TRUE;
	}
	return FALSE;
}
int32_t field::execute_operation(uint16_t step, effect* triggering_effect, uint8_t triggering_player) {
	if(!triggering_effect->operation) {
		core.solving_event.splice(core.solving_event.begin(), core.sub_solving_event);
		pduel->lua->params.clear();
		return TRUE;
	}
	if (step == 0) {
		core.solving_event.splice(core.solving_event.begin(), core.sub_solving_event);
		const tevent& e = core.solving_event.front();
		pduel->lua->add_param<PARAM_TYPE_INT>(e.reason_player, true);
		pduel->lua->add_param<PARAM_TYPE_INT>(e.reason, true);
		pduel->lua->add_param<PARAM_TYPE_EFFECT>(e.reason_effect , true);
		pduel->lua->add_param<PARAM_TYPE_INT>(e.event_value, true);
		pduel->lua->add_param<PARAM_TYPE_INT>(e.event_player, true);
		pduel->lua->add_param<PARAM_TYPE_GROUP>(e.event_cards , true);
		pduel->lua->add_param<PARAM_TYPE_INT>(triggering_player, true);
		pduel->lua->add_param<PARAM_TYPE_EFFECT>(triggering_effect, true);
		if(core.check_level == 0) {
			core.shuffle_deck_check[0] = FALSE;
			core.shuffle_deck_check[1] = FALSE;
			core.shuffle_hand_check[0] = FALSE;
			core.shuffle_hand_check[1] = FALSE;
		}
		core.units.begin()->arg2 = core.shuffle_check_disabled;
		core.shuffle_check_disabled = FALSE;
		++core.check_level;
	}
	core.reason_effect = triggering_effect;
	core.reason_player = triggering_player;
	uint32_t count = static_cast<uint32_t>(pduel->lua->params.size());
	lua_Integer yield_value = 0;
	int32_t result = pduel->lua->call_coroutine(triggering_effect->operation, count, &yield_value, step);
	returns.set<int32_t>(0, static_cast<int32_t>(yield_value));
	if (result != COROUTINE_YIELD) {
		core.reason_effect = 0;
		core.reason_player = PLAYER_NONE;
		--core.check_level;
		if(core.check_level == 0) {
			if(core.shuffle_hand_check[0])
				shuffle(0, LOCATION_HAND);
			if(core.shuffle_hand_check[1])
				shuffle(1, LOCATION_HAND);
			if(core.shuffle_deck_check[0])
				shuffle(0, LOCATION_DECK);
			if(core.shuffle_deck_check[1])
				shuffle(1, LOCATION_DECK);
			//cost[0].count = 0;
			//cost[1].count = 0;
			//cost[0].amount = 0;
			//cost[1].amount = 0;
		}
		core.shuffle_check_disabled = core.units.begin()->arg2;
		return TRUE;
	}
	return FALSE;
}
int32_t field::execute_target(uint16_t step, effect* triggering_effect, uint8_t triggering_player) {
	if(!triggering_effect->target) {
		core.solving_event.splice(core.solving_event.begin(), core.sub_solving_event);
		pduel->lua->params.clear();
		return TRUE;
	}
	if (step == 0) {
		core.solving_event.splice(core.solving_event.begin(), core.sub_solving_event);
		const tevent& e = core.solving_event.front();
		pduel->lua->add_param<PARAM_TYPE_INT>(1, true);
		pduel->lua->add_param<PARAM_TYPE_INT>(e.reason_player, true);
		pduel->lua->add_param<PARAM_TYPE_INT>(e.reason, true);
		pduel->lua->add_param<PARAM_TYPE_EFFECT>(e.reason_effect , true);
		pduel->lua->add_param<PARAM_TYPE_INT>(e.event_value, true);
		pduel->lua->add_param<PARAM_TYPE_INT>(e.event_player, true);
		pduel->lua->add_param<PARAM_TYPE_GROUP>(e.event_cards , true);
		pduel->lua->add_param<PARAM_TYPE_INT>(triggering_player, true);
		pduel->lua->add_param<PARAM_TYPE_EFFECT>(triggering_effect, true);
		if(core.check_level == 0) {
			core.shuffle_deck_check[0] = FALSE;
			core.shuffle_deck_check[1] = FALSE;
			core.shuffle_hand_check[0] = FALSE;
			core.shuffle_hand_check[1] = FALSE;
		}
		core.units.begin()->arg2 = core.shuffle_check_disabled;
		core.shuffle_check_disabled = FALSE;
		++core.check_level;
	}
	core.reason_effect = triggering_effect;
	core.reason_player = triggering_player;
	uint32_t count = static_cast<uint32_t>(pduel->lua->params.size());
	lua_Integer yield_value = 0;
	int32_t result = pduel->lua->call_coroutine(triggering_effect->target, count, &yield_value, step);
	returns.set<int32_t>(0, static_cast<int32_t>(yield_value));
	if (result != COROUTINE_YIELD) {
		core.reason_effect = 0;
		core.reason_player = PLAYER_NONE;
		--core.check_level;
		if(core.check_level == 0) {
			if(core.shuffle_hand_check[0])
				shuffle(0, LOCATION_HAND);
			if(core.shuffle_hand_check[1])
				shuffle(1, LOCATION_HAND);
			if(core.shuffle_deck_check[0])
				shuffle(0, LOCATION_DECK);
			if(core.shuffle_deck_check[1])
				shuffle(1, LOCATION_DECK);
		}
		core.shuffle_check_disabled = core.units.begin()->arg2;
		return TRUE;
	}
	return FALSE;
}
void field::raise_event(card* event_card, uint32_t event_code, effect* reason_effect, uint32_t reason, uint8_t reason_player, uint8_t event_player, uint32_t event_value) {
	tevent new_event;
	new_event.trigger_card = 0;
	if (event_card) {
		group* pgroup = pduel->new_group(event_card);
		pgroup->is_readonly = TRUE;
		new_event.event_cards = pgroup;
	} else
		new_event.event_cards = 0;
	new_event.event_code = event_code;
	new_event.reason_effect = reason_effect;
	new_event.reason = reason;
	new_event.reason_player = reason_player;
	new_event.event_player = event_player;
	new_event.event_value = event_value;
	new_event.global_id = infos.event_id;
	core.queue_event.push_back(new_event);
}
void field::raise_event(card_set* event_cards, uint32_t event_code, effect* reason_effect, uint32_t reason, uint8_t reason_player, uint8_t event_player, uint32_t event_value) {
	core.queue_event.emplace_back();
	tevent& new_event = core.queue_event.back();
	new_event.trigger_card = 0;
	if (event_cards) {
		group* pgroup = pduel->new_group(*event_cards);
		pgroup->is_readonly = TRUE;
		new_event.event_cards = pgroup;
	} else
		new_event.event_cards = 0;
	new_event.event_code = event_code;
	new_event.reason_effect = reason_effect;
	new_event.reason = reason;
	new_event.reason_player = reason_player;
	new_event.event_player = event_player;
	new_event.event_value = event_value;
	new_event.global_id = infos.event_id;
}
void field::raise_single_event(card* trigger_card, card_set* event_cards, uint32_t event_code, effect* reason_effect, uint32_t reason, uint8_t reason_player, uint8_t event_player, uint32_t event_value) {
	core.single_event.emplace_back();
	tevent& new_event = core.single_event.back();
	new_event.trigger_card = trigger_card;
	if (event_cards) {
		group* pgroup = pduel->new_group(*event_cards);
		pgroup->is_readonly = TRUE;
		new_event.event_cards = pgroup;
	} else
		new_event.event_cards = 0;
	new_event.event_code = event_code;
	new_event.reason_effect = reason_effect;
	new_event.reason = reason;
	new_event.reason_player = reason_player;
	new_event.event_player = event_player;
	new_event.event_value = event_value;
	new_event.global_id = infos.event_id;
}
int32_t field::check_event(uint32_t code, tevent* pe) {
	for(const auto& ev : core.point_event) {
		if(ev.event_code == code) {
			if(pe)
				*pe = ev;
			return TRUE;
		}
	}
	for(const auto& ev : core.instant_event) {
		if(ev.event_code == code) {
			if(pe)
				*pe = ev;
			return TRUE;
		}
	}
	return FALSE;
}
int32_t field::check_event_c(effect* peffect, uint8_t playerid, int32_t neglect_con, int32_t neglect_cost, int32_t copy_info, tevent* pe) {
	if(peffect->code == EVENT_FREE_CHAIN) {
		return peffect->is_activate_ready(core.reason_effect, playerid, nil_event, neglect_con, neglect_cost, FALSE);
	}
	for(const auto& ev : core.point_event) {
		if(ev.event_code == peffect->code &&
		        peffect->is_activate_ready(core.reason_effect, playerid, ev, neglect_con, neglect_cost, FALSE)) {
			if(pe)
				*pe = ev;
			if(copy_info && !pduel->lua->no_action && core.current_chain.size()) {
				core.current_chain.back().evt = ev;
			}
			return TRUE;
		}
	}
	for(const auto& ev : core.instant_event) {
		if(ev.event_code == peffect->code &&
		        peffect->is_activate_ready(core.reason_effect, playerid, ev, neglect_con, neglect_cost, FALSE)) {
			if(pe)
				*pe = ev;
			if(copy_info && !pduel->lua->no_action && core.current_chain.size()) {
				core.current_chain.back().evt = ev;
			}
			return TRUE;
		}
	}
	return FALSE;
}
int32_t field::check_hint_timing(effect* peffect) {
	int32_t p = peffect->get_handler_player();
	if(p == 0)
		return (peffect->hint_timing[0] & core.hint_timing[0]) || (peffect->hint_timing[1] & core.hint_timing[1]);
	else
		return (peffect->hint_timing[0] & core.hint_timing[1]) || (peffect->hint_timing[1] & core.hint_timing[0]);
}
// core.spe_effect[p]: # of optional trigger effects, activate/quick effect with hints
int32_t field::process_phase_event(int16_t step, int32_t phase) {
	switch(step) {
	case 0: {
		if((phase == PHASE_DRAW && (core.force_turn_end || is_player_affected_by_effect(infos.turn_player, EFFECT_SKIP_DP)))
				|| (phase == PHASE_STANDBY && (core.force_turn_end || is_player_affected_by_effect(infos.turn_player, EFFECT_SKIP_SP)))
				|| (phase == PHASE_BATTLE_START && (core.force_turn_end || is_player_affected_by_effect(infos.turn_player, EFFECT_SKIP_BP)))
				|| (phase == PHASE_BATTLE && (core.force_turn_end || is_player_affected_by_effect(infos.turn_player, EFFECT_SKIP_BP)))
				|| (phase == PHASE_END && (core.force_turn_end || is_player_affected_by_effect(infos.turn_player, EFFECT_SKIP_EP)))) {
			core.units.begin()->step = 24;
			return FALSE;
		}
		int32_t phase_event = EVENT_PHASE + phase;
		nil_event.event_code = phase_event;
		nil_event.event_player = infos.turn_player;
		int32_t check_player = infos.turn_player;
		if(core.units.begin()->arg2 & 0x2)
			check_player = 1 - infos.turn_player;
		core.select_chains.clear();
		int32_t tf_count = 0, to_count = 0, fc_count = 0, cn_count = 0;
		auto pr = effects.trigger_f_effect.equal_range(phase_event);
		for(auto eit = pr.first; eit != pr.second;) {
			effect* peffect = eit->second;
			++eit;
			peffect->set_activate_location();
			if(!peffect->is_activateable(check_player, nil_event))
				continue;
			peffect->id = infos.field_id++;
			core.select_chains.emplace_back();
			core.select_chains.back().triggering_effect = peffect;
			++tf_count;
		}
		pr = effects.continuous_effect.equal_range(phase_event);
		for(auto eit = pr.first; eit != pr.second;) {
			effect* peffect = eit->second;
			++eit;
			//effects.continuous_effect may be changed in is_activateable (e.g. Rescue Cat)
			if(peffect->get_handler_player() != check_player || !peffect->is_activateable(check_player, nil_event))
				continue;
			peffect->id = infos.field_id++;
			core.select_chains.emplace_back();
			core.select_chains.back().triggering_effect = peffect;
			++cn_count;
		}
		//all effects taking control non-permanently are only until End Phase, not until Turn end
		for(auto* peffect : effects.pheff) {
			if(peffect->code != EFFECT_SET_CONTROL)
				continue;
			if(!(peffect->reset_flag & phase))
				continue;
			uint8_t pid = peffect->get_handler_player();
			if(pid != check_player)
				continue;
			uint8_t tp = infos.turn_player;
			if(!(((peffect->reset_flag & RESET_SELF_TURN) && pid == tp) || ((peffect->reset_flag & RESET_OPPO_TURN) && pid != tp)))
				continue;
			if(peffect->reset_count != 1)
				continue;
			card* phandler = peffect->get_handler();
			if(peffect->get_value(phandler) != phandler->current.controler)
				continue;
			core.select_chains.emplace_back();
			core.select_chains.back().triggering_effect = peffect;
			++cn_count;
		}
		core.spe_effect[check_player] = 0;
		if(!core.hand_adjusted) {
			pr = effects.trigger_o_effect.equal_range(phase_event);
			for(auto eit = pr.first; eit != pr.second;) {
				effect* peffect = eit->second;
				++eit;
				peffect->set_activate_location();
				if(!peffect->is_activateable(check_player, nil_event))
					continue;
				peffect->id = infos.field_id++;
				core.select_chains.emplace_back();
				core.select_chains.back().triggering_effect = peffect;
				++to_count;
				++core.spe_effect[check_player];
			}
			if(phase == PHASE_DRAW)
				core.hint_timing[infos.turn_player] = TIMING_DRAW_PHASE;
			else if(phase == PHASE_STANDBY)
				core.hint_timing[infos.turn_player] = TIMING_STANDBY_PHASE;
			else if(phase == PHASE_BATTLE_START)
				core.hint_timing[infos.turn_player] = TIMING_BATTLE_START;
			else if(phase == PHASE_BATTLE)
				core.hint_timing[infos.turn_player] = TIMING_BATTLE_END;
			else
				core.hint_timing[infos.turn_player] = TIMING_END_PHASE;
			pr = effects.activate_effect.equal_range(EVENT_FREE_CHAIN);
			for(auto eit = pr.first; eit != pr.second;) {
				effect* peffect = eit->second;
				++eit;
				peffect->set_activate_location();
				if(!peffect->is_chainable(check_player) || !peffect->is_activateable(check_player, nil_event))
					continue;
				peffect->id = infos.field_id++;
				core.select_chains.emplace_back();
				core.select_chains.back().triggering_effect = peffect;
				if(check_hint_timing(peffect) || check_cteffect_hint(peffect, check_player))
					++core.spe_effect[check_player];
				++fc_count;
			}
			pr = effects.quick_o_effect.equal_range(EVENT_FREE_CHAIN);
			for(auto eit = pr.first; eit != pr.second;) {
				effect* peffect = eit->second;
				++eit;
				peffect->set_activate_location();
				if(!peffect->is_chainable(check_player) || !peffect->is_activateable(check_player, nil_event))
					continue;
				peffect->id = infos.field_id++;
				core.select_chains.emplace_back();
				core.select_chains.back().triggering_effect = peffect;
				if(check_hint_timing(peffect))
					++core.spe_effect[check_player];
				++fc_count;
			}
			pr = effects.continuous_effect.equal_range(EVENT_FREE_CHAIN);
			for(auto eit = pr.first; eit != pr.second;) {
				effect* peffect = eit->second;
				++eit;
				if(peffect->get_handler_player() != check_player || !peffect->is_activateable(check_player, nil_event))
					continue;
				peffect->id = infos.field_id++;
				core.select_chains.emplace_back();
				core.select_chains.back().triggering_effect = peffect;
				++fc_count;
			}
		}
		if(core.select_chains.size() == 0) {
			returns.set<int32_t>(0, -1);
			core.units.begin()->step = 1;
			return FALSE;
		} else if(tf_count == 0 && cn_count == 1 && to_count == 0 && fc_count == 0) {
			returns.set<int32_t>(0, 0);
			core.units.begin()->step = 1;
			return FALSE;
		} else {
			auto message = pduel->new_message(MSG_HINT);
			message->write<uint8_t>(HINT_EVENT);
			message->write<uint8_t>(check_player);
			if(infos.phase == PHASE_DRAW)
				message->write<uint64_t>(20);
			else if(infos.phase == PHASE_STANDBY)
				message->write<uint64_t>(21);
			else if(infos.phase == PHASE_BATTLE_START)
				message->write<uint64_t>(28);
			else if(infos.phase == PHASE_BATTLE)
				message->write<uint64_t>(25);
			else
				message->write<uint64_t>(26);
			if(tf_count == 0 && to_count == 1 && fc_count == 0 && cn_count == 0) {
				add_process(PROCESSOR_SELECT_EFFECTYN, 0, 0, (group*)core.select_chains[0].triggering_effect->get_handler(), check_player, 0);
				return FALSE;
			} else {
				add_process(PROCESSOR_SELECT_CHAIN, 0, 0, 0, check_player, core.spe_effect[check_player] | (tf_count + cn_count ? 0x10000 : 0));
				core.units.begin()->step = 1;
				return FALSE;
			}
		}
		return FALSE;
	}
	case 1: {
		returns.set<int32_t>(0, returns.at<int32_t>(0) - 1);
		return FALSE;
	}
	case 2: {
		int32_t priority_passed = core.units.begin()->arg2 & 0x1;
		int32_t is_opponent = core.units.begin()->arg2 & 0x2;
		if(returns.at<int32_t>(0) == -1) {
			if(priority_passed)
				core.units.begin()->step = 19;
			else {
				priority_passed = 1;
				is_opponent ^= 0x2;
				core.units.begin()->arg2 = is_opponent | priority_passed;
				core.units.begin()->step = -1;
			}
			return FALSE;
		}
		priority_passed = 0;
		core.units.begin()->arg2 = is_opponent | priority_passed;
		chain& selected_chain = core.select_chains[returns.at<int32_t>(0)];
		effect* peffect = selected_chain.triggering_effect;
		card* phandler = peffect->get_handler();
		if(!(peffect->type & EFFECT_TYPE_ACTIONS)) {
			if(peffect->is_flag(EFFECT_FLAG_FIELD_ONLY))
				remove_effect(peffect);
			else
				peffect->handler->remove_effect(peffect);
			adjust_all();
			core.units.begin()->step = 3;
		} else if(!(peffect->type & EFFECT_TYPE_CONTINUOUS)) {
			int32_t check_player = infos.turn_player;
			if(is_opponent)
				check_player = 1 - infos.turn_player;
			core.new_chains.emplace_back(std::move(selected_chain));
			chain& newchain = core.new_chains.back();
			newchain.flag = 0;
			newchain.chain_id = infos.field_id++;
			newchain.evt = nil_event;
			newchain.set_triggering_state(phandler);
			newchain.triggering_player = check_player;
			phandler->set_status(STATUS_CHAINING, TRUE);
			peffect->dec_count(check_player);
			core.select_chains.clear();
			add_process(PROCESSOR_ADD_CHAIN, 0, 0, 0, 0, 0);
			if(is_flag(DUEL_INVERTED_QUICK_PRIORITY))
				add_process(PROCESSOR_QUICK_EFFECT, 0, 0, 0, FALSE, check_player);
			else
				add_process(PROCESSOR_QUICK_EFFECT, 0, 0, 0, FALSE, 1 - check_player);
			infos.priorities[0] = 0;
			infos.priorities[1] = 0;
		} else {
			core.select_chains.clear();
			solve_continuous(peffect->get_handler_player(), peffect, nil_event);
			core.units.begin()->step = 3;
		}
		return FALSE;
	}
	case 3: {
		for(auto& ch_lim : core.chain_limit)
			luaL_unref(pduel->lua->lua_state, LUA_REGISTRYINDEX, ch_lim.function);
		core.chain_limit.clear();
		for(auto& ch : core.current_chain)
			ch.triggering_effect->get_handler()->set_status(STATUS_CHAINING, FALSE);
		add_process(PROCESSOR_SOLVE_CHAIN, 0, 0, 0, FALSE, 0);
		core.units.begin()->step = -1;
		return FALSE;
	}
	case 4: {
		adjust_instant();
		add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, 0, 0);
		core.units.begin()->step = -1;
		return FALSE;
	}
	case 20: {
		if(phase != PHASE_END) {
			core.units.begin()->step = 24;
			return FALSE;
		}
		int32_t limit = 6;
		effect_set eset;
		filter_player_effect(infos.turn_player, EFFECT_HAND_LIMIT, &eset);
		if(eset.size())
			limit = eset.back()->get_value();
		int32_t hd = static_cast<int32_t>(player[infos.turn_player].list_hand.size());
		if(hd <= limit || is_flag(DUEL_NO_HAND_LIMIT)) {
			core.units.begin()->step = 24;
			return FALSE;
		}
		core.select_cards.clear();
		for(auto& pcard : player[infos.turn_player].list_hand)
			core.select_cards.push_back(pcard);
		auto message = pduel->new_message(MSG_HINT);
		message->write<uint8_t>(HINT_SELECTMSG);
		message->write<uint8_t>(infos.turn_player);
		message->write<uint64_t>(501);
		add_process(PROCESSOR_SELECT_CARD, 0, 0, 0, infos.turn_player, hd - limit + ((hd - limit) << 16));
		return FALSE;
	}
	case 21: {
		if(return_cards.list.size())
			send_to(card_set{ return_cards.list.begin(), return_cards.list.end() }, 0, REASON_RULE + REASON_DISCARD + REASON_ADJUST, infos.turn_player, PLAYER_NONE, LOCATION_GRAVE, 0, POS_FACEUP);
		return FALSE;
	}
	case 22: {
		core.hand_adjusted = TRUE;
		add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, 0, 0);
		core.units.begin()->step = -1;
		core.units.begin()->arg2 = 0;
		return FALSE;
	}
	case 25: {
		core.hint_timing[infos.turn_player] = 0;
		reset_phase(phase);
		adjust_all();
		return FALSE;
	}
	case 26: {
		core.quick_f_chain.clear();
		core.instant_event.clear();
		core.point_event.clear();
		core.delayed_activate_event.clear();
		core.full_event.clear();
		/*if(core.set_forced_attack) {
			core.set_forced_attack = false;
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);
		}*/
		return TRUE;
	}
	}
	/*if(core.set_forced_attack) {
		core.set_forced_attack = false;
		add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);
	}*/
	return TRUE;
}
// core.ignition_priority_chains: used in step 8 (obsolete ignition effect ruling)
int32_t field::process_point_event(int16_t step, int32_t skip_trigger, int32_t skip_freechain, int32_t skip_new) {
	switch(step) {
	case 0: {
		core.select_chains.clear();
		core.point_event.splice(core.point_event.end(), core.instant_event);
		if(skip_trigger) {
			core.units.begin()->step = 7;
			return FALSE;
		}
		core.new_fchain_s.splice(core.new_fchain_s.begin(), core.new_fchain);
		core.new_ochain_s.splice(core.new_ochain_s.begin(), core.new_ochain);
		core.full_event.splice(core.full_event.end(), core.delayed_activate_event);
		core.delayed_quick.clear();
		core.delayed_quick_tmp.swap(core.delayed_quick);
		core.current_player = infos.turn_player;
		core.units.begin()->step = 1;
		return FALSE;
	}
	case 1: {
		return FALSE;
	}
	case 2: {
		//forced trigger
		core.select_chains.clear();
		for (auto clit = core.new_fchain_s.begin(); clit != core.new_fchain_s.end(); ) {
			effect* peffect = clit->triggering_effect;
			card* phandler = peffect->get_handler();
			if(phandler->is_has_relation(*clit)) //work around: position and control should be refreshed before raising event
				clit->set_triggering_state(phandler);
			uint8_t tp = clit->triggering_player;
			if((!clit->was_just_sent || phandler->current.location == LOCATION_HAND)
			   && check_trigger_effect(*clit)
			   && peffect->is_chainable(tp)
			   && peffect->is_activateable(tp, clit->evt, TRUE, FALSE, FALSE, is_flag(DUEL_TRIGGER_WHEN_PRIVATE_KNOWLEDGE), is_flag(DUEL_TRIGGER_WHEN_PRIVATE_KNOWLEDGE))) {
				if(tp == core.current_player)
					core.select_chains.push_back(*clit);
			} else {
				peffect->active_type = 0;
				core.new_fchain_s.erase(clit++);
				continue;
			}
			++clit;
		}
		//TCG SEGOC
		if(core.select_chains.size() == 0) {
			returns.set<int32_t>(0, -1);
		} else {
			if(is_flag(DUEL_TCG_SEGOC_FIRSTTRIGGER)) { //tcg segoc
				std::sort(core.select_chains.begin(), core.select_chains.end(), [](const chain& c1, const chain& c2) {return c1.event_id < c2.event_id; });
				auto pred = [id = core.select_chains.front().event_id](const chain& ch) {
					return ch.event_id == id;
				};
				auto endit = std::find_if_not(core.select_chains.begin(), core.select_chains.end(), pred);
				core.select_chains.erase(endit, core.select_chains.end());
			}
			if(core.select_chains.size() == 1) {
				returns.set<int32_t>(0, 0);
			} else {
				add_process(PROCESSOR_SELECT_CHAIN, 0, 0, 0, core.current_player, 0x7f | 0x10000);
			}
		}
		return FALSE;
	}
	case 3: {
		if(returns.at<int32_t>(0) == -1) {
			if(core.new_fchain_s.size()) {
				core.current_player = 1 - infos.turn_player;
				core.units.begin()->step = 1;
			} else {
				core.current_player = infos.turn_player;
			}
			return FALSE;
		}
		chain& newchain = core.select_chains[returns.at<int32_t>(0)];
		effect* peffect = newchain.triggering_effect;
		uint8_t tp = newchain.triggering_player;
		peffect->get_handler()->set_status(STATUS_CHAINING, TRUE);
		peffect->dec_count(tp);
		auto chain_id = newchain.chain_id;
		core.new_chains.push_back(std::move(newchain));
		add_process(PROCESSOR_ADD_CHAIN, 0, 0, 0, 0, 0);
		core.new_fchain_s.remove_if([chain_id](const chain& ch) { return ch.chain_id == chain_id; });
		core.units.begin()->step = 1;
		return FALSE;
	}
	case 4: {
		//optional trigger
		core.select_chains.clear();
		for (auto clit = core.new_ochain_s.begin(); clit != core.new_ochain_s.end(); ) {
			effect* peffect = clit->triggering_effect;
			card* phandler = peffect->get_handler();
			if(phandler->is_has_relation(*clit)) //work around: position and control should be refreshed before raising event
				clit->set_triggering_state(phandler);
			if(!peffect->is_flag(EFFECT_FLAG_FIELD_ONLY) && (peffect->type & EFFECT_TYPE_FIELD)
				&& (peffect->range & LOCATION_HAND) && phandler->current.location == LOCATION_HAND) {
				if(!phandler->is_has_relation(*clit) && peffect->is_condition_check(phandler->current.controler, clit->evt))
					phandler->create_relation(*clit);
				peffect->set_activate_location();
				clit->triggering_player = phandler->current.controler;
				clit->set_triggering_state(phandler);
			}
			uint8_t tp = clit->triggering_player;
			if((!clit->was_just_sent || phandler->current.location == LOCATION_HAND)
			   && (is_flag(DUEL_TRIGGER_WHEN_PRIVATE_KNOWLEDGE) || check_nonpublic_trigger(*clit))
			   && check_trigger_effect(*clit)
			   && peffect->is_chainable(tp)
			   && peffect->is_activateable(tp, clit->evt, TRUE, FALSE, FALSE, is_flag(DUEL_TRIGGER_WHEN_PRIVATE_KNOWLEDGE), is_flag(DUEL_TRIGGER_WHEN_PRIVATE_KNOWLEDGE))
			   && check_spself_from_hand_trigger(*clit)) {
				if(tp == core.current_player)
					core.select_chains.push_back(*clit);
			} else {
				peffect->active_type = 0;
				core.new_ochain_s.erase(clit++);
				continue;
			}
			++clit;
		}
		//TCG SEGOC
		if(core.select_chains.size() == 0) {
			returns.set<int32_t>(0, -2);
			core.units.begin()->step = 5;
			return FALSE;
		} else {
			if(is_flag(DUEL_TCG_SEGOC_NONPUBLIC))
				core.new_ochain_h.clear();
			//tcg segoc
			if(is_flag(DUEL_TCG_SEGOC_FIRSTTRIGGER)) {
				std::sort(core.select_chains.begin(), core.select_chains.end(), [](const chain& c1, const chain& c2) {return c1.event_id < c2.event_id; });
				auto pred = [id = core.select_chains.front().event_id](const chain& ch) {
					return ch.event_id == id;
				};
				auto endit = std::find_if_not(core.select_chains.begin(), core.select_chains.end(), pred);
				core.select_chains.erase(endit, core.select_chains.end());
			}
			if(core.select_chains.size() == 1 && !core.current_chain.size()) {
				add_process(PROCESSOR_SELECT_EFFECTYN, 0, 0, (group*)core.select_chains[0].triggering_effect->get_handler(), core.current_player, 221);
				return FALSE;
			} else {
				add_process(PROCESSOR_SELECT_CHAIN, 0, 0, 0, core.current_player, 0x7f);
				core.units.begin()->step = 5;
				return FALSE;
			}
		}
		return FALSE;
	}
	case 5: {
		returns.set<int32_t>(0, returns.at<int32_t>(0) - 1);
		return FALSE;
	}
	case 6: {
		const auto ret = returns.at<int32_t>(0);
		if(ret == -2 || (ret == -1 && !is_flag(DUEL_TCG_SEGOC_FIRSTTRIGGER))) {
			for(const auto& ch : core.select_chains) {
				ch.triggering_effect->active_type = 0;
				core.new_ochain_s.remove_if([chain_id = ch.chain_id](const chain& ch) { return ch.chain_id == chain_id; });
			}
			if(core.new_ochain_s.size()) {
				core.current_player = 1 - infos.turn_player;
				core.units.begin()->step = 3;
			} else {
				core.current_player = infos.turn_player;
				core.units.begin()->step = 6;
			}
			return FALSE;
		} else if(ret == -1) {
			//TCG SEGOC
			auto discardedid = core.select_chains[0].event_id;
			core.new_ochain_s.remove_if([&](const chain& ch) { return ch.event_id == discardedid && ch.triggering_player == core.current_player; });
			core.units.begin()->step = 3;
			return FALSE;
		}
		chain& newchain = core.select_chains[returns.at<int32_t>(0)];
		effect* peffect = newchain.triggering_effect;
		uint8_t tp = newchain.triggering_player;
		peffect->get_handler()->set_status(STATUS_CHAINING, TRUE);
		peffect->dec_count(tp);
		auto chain_id = newchain.chain_id;
		core.new_chains.push_back(std::move(newchain));
		add_process(PROCESSOR_ADD_CHAIN, 0, 0, 0, 0, 0);
		core.new_ochain_s.remove_if([chain_id](const chain& ch) { return ch.chain_id == chain_id; });
		core.new_ochain_h.remove_if([chain_id](const chain& ch) { return ch.chain_id == chain_id; });
		core.units.begin()->step = 3;
		return FALSE;
	}
	case 7: {
		core.select_chains.clear();
		return FALSE;
	}
	case 8: {
		if(!is_flag(DUEL_OBSOLETE_IGNITION) || (infos.phase != PHASE_MAIN1 && infos.phase != PHASE_MAIN2))
			return FALSE;
		// Obsolete ignition effect ruling
		tevent _e;
		if(core.current_chain.size() == 0 &&
		   (((check_event(EVENT_SUMMON_SUCCESS, &_e) || check_event(EVENT_SPSUMMON_SUCCESS, &_e) ||
			 check_event(EVENT_FLIP_SUMMON_SUCCESS, &_e)) && _e.reason_player == infos.turn_player)
		   || check_event(EVENT_CHAIN_END))) {
			chain newchain;
			{
				newchain.evt.event_cards = 0;
				newchain.evt.event_value = 0;
				newchain.evt.event_player = PLAYER_NONE;
				newchain.evt.reason_effect = 0;
				newchain.evt.reason = 0;
				newchain.evt.reason_player = PLAYER_NONE;
			}
			newchain.flag = 0;
			newchain.triggering_player = infos.turn_player;
			for(auto eit = effects.ignition_effect.begin(); eit != effects.ignition_effect.end();) {
				effect* peffect = eit->second;
				++eit;
				card* phandler = peffect->get_handler();
				newchain.evt.event_code = peffect->code;
				if(phandler->current.location == LOCATION_MZONE && peffect->is_chainable(infos.turn_player)
				        && peffect->is_activateable(infos.turn_player, newchain.evt)) {
					newchain.chain_id = infos.field_id++;
					newchain.triggering_effect = peffect;
					newchain.set_triggering_state(phandler);
					core.ignition_priority_chains.push_back(newchain);
				}
			}
		}
		return FALSE;
	}
	case 9: {
		infos.priorities[0] = 0;
		infos.priorities[1] = 0;
		if(core.current_chain.size() == 0) {
			if(!core.hand_adjusted) {
				if(is_flag(DUEL_INVERTED_QUICK_PRIORITY))
					add_process(PROCESSOR_QUICK_EFFECT, 0, 0, 0, skip_freechain, 1 - infos.turn_player);
				else
					add_process(PROCESSOR_QUICK_EFFECT, 0, 0, 0, skip_freechain, infos.turn_player);
			}
		} else
			add_process(PROCESSOR_QUICK_EFFECT, 0, 0, 0, skip_freechain, 1 - core.current_chain.back().triggering_player);
		return FALSE;
	}
	case 10: {
		core.new_ochain_h.clear();
		core.full_event.clear();
		core.delayed_quick.clear();
		for(auto& ch_lim : core.chain_limit)
			luaL_unref(pduel->lua->lua_state, LUA_REGISTRYINDEX, ch_lim.function);
		core.chain_limit.clear();
		if(core.current_chain.size()) {
			for(auto& ch : core.current_chain)
				ch.triggering_effect->get_handler()->set_status(STATUS_CHAINING, FALSE);
			add_process(PROCESSOR_SOLVE_CHAIN, 0, 0, 0, skip_trigger | ((skip_freechain | skip_new) << 8), skip_new);
		} else {
			core.used_event.splice(core.used_event.end(), core.point_event);
			for(auto& ch_lim_p : core.chain_limit_p)
				luaL_unref(pduel->lua->lua_state, LUA_REGISTRYINDEX, ch_lim_p.function);
			core.chain_limit_p.clear();
			reset_chain();
			returns.set<int32_t>(0, FALSE);
		}
		if(core.set_forced_attack) {
			core.set_forced_attack = false;
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);
		}
		return TRUE;
	}
	case 30: {
		int32_t check_player = infos.turn_player;
		nil_event.event_code = EVENT_FREE_CHAIN;
		core.select_chains.clear();
		core.spe_effect[check_player] = 0;
		auto pr = effects.continuous_effect.equal_range(EVENT_FREE_CHAIN);
		for(auto eit = pr.first; eit != pr.second;) {
			effect* peffect = eit->second;
			++eit;
			if(peffect->get_handler_player() == check_player && peffect->is_activateable(check_player, nil_event)) {
				core.select_chains.emplace_back();
				core.select_chains.back().triggering_effect = peffect;
				++core.spe_effect[check_player];
			}
		}
		if(!core.select_chains.empty())
			add_process(PROCESSOR_SELECT_CHAIN, 0, 0, 0, check_player, core.spe_effect[check_player]);
		else
			core.units.begin()->step = 31;
		return FALSE;
	}
	case 31: {
		if(returns.at<int32_t>(0) == -1)
			return FALSE;
		const chain& newchain = core.select_chains[returns.at<int32_t>(0)];
		effect* peffect = newchain.triggering_effect;
		core.select_chains.clear();
		solve_continuous(peffect->get_handler_player(), peffect, nil_event);
		core.units.begin()->step = 29;
		return FALSE;
	}
	case 32: {
		int32_t check_player = 1 - infos.turn_player;
		nil_event.event_code = EVENT_FREE_CHAIN;
		core.select_chains.clear();
		core.spe_effect[check_player] = 0;
		auto pr = effects.continuous_effect.equal_range(EVENT_FREE_CHAIN);
		for(auto eit = pr.first; eit != pr.second;) {
			effect* peffect = eit->second;
			++eit;
			if(peffect->get_handler_player() == check_player && peffect->is_activateable(check_player, nil_event)) {
				core.select_chains.emplace_back();
				core.select_chains.back().triggering_effect = peffect;
				++core.spe_effect[check_player];
			}
		}
		if(!core.select_chains.empty())
			add_process(PROCESSOR_SELECT_CHAIN, 0, 0, 0, check_player, core.spe_effect[check_player]);
		else
			core.units.begin()->step = -1;
		return FALSE;
	}
	case 33: {
		if(returns.at<int32_t>(0) == -1) {
			core.units.begin()->step = -1;
			return FALSE;
		}
		const chain& newchain = core.select_chains[returns.at<int32_t>(0)];
		effect* peffect = newchain.triggering_effect;
		core.select_chains.clear();
		solve_continuous(peffect->get_handler_player(), peffect, nil_event);
		core.units.begin()->step = 31;
		return FALSE;
	}
	}
	if(core.set_forced_attack) {
		core.set_forced_attack = false;
		add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);
	}
	return TRUE;
}
int32_t field::process_quick_effect(int16_t step, int32_t skip_freechain, uint8_t priority) {
	switch(step) {
	case 0: {
		uint8_t check_player = infos.turn_player;
		if(core.units.begin()->arg3)
			check_player = 1 - infos.turn_player;
		core.select_chains.clear();
		for(auto ifit = core.quick_f_chain.begin(); ifit != core.quick_f_chain.end(); ) {
			effect* peffect = ifit->first;
			card* phandler = peffect->get_handler();
			if(peffect->is_chainable(ifit->second.triggering_player) && peffect->check_count_limit(ifit->second.triggering_player)
					&& phandler->is_has_relation(ifit->second)) {
				if(ifit->second.triggering_player == check_player)
					core.select_chains.push_back(ifit->second);
			} else {
				core.quick_f_chain.erase(ifit++);
				continue;
			}
			++ifit;
		}
		if(core.select_chains.size() == 0)
			returns.set<int32_t>(0, -1);
		else if(core.select_chains.size() == 1)
			returns.set<int32_t>(0, 0);
		else
			add_process(PROCESSOR_SELECT_CHAIN, 0, 0, 0, check_player, 0x10000);
		return FALSE;
	}
	case 1: {
		if(returns.at<int32_t>(0) == -1) {
			if(core.quick_f_chain.size()) {
				core.units.begin()->arg3 = TRUE;
				core.units.begin()->step = -1;
			} else if(core.new_chains.size()) {
				add_process(PROCESSOR_ADD_CHAIN, 0, 0, 0, 0, 0);
				add_process(PROCESSOR_QUICK_EFFECT, 0, 0, 0, FALSE, 1 - core.new_chains.back().triggering_player);
				infos.priorities[0] = 0;
				infos.priorities[1] = 0;
				/*if(core.set_forced_attack) {
					core.set_forced_attack = false;
					add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);
				}*/
				return TRUE;
			}
			return FALSE;
		}
		chain& newchain = core.select_chains[returns.at<int32_t>(0)];
		effect* peffect = newchain.triggering_effect;
		uint8_t tp = newchain.triggering_player;
		peffect->get_handler()->set_status(STATUS_CHAINING, TRUE);
		peffect->dec_count(tp);
		core.new_chains.push_back(std::move(newchain));
		core.quick_f_chain.erase(peffect);
		core.units.begin()->step = -1;
		return FALSE;
	}
	case 2: {
		if(core.ignition_priority_chains.size())
			core.select_chains.swap(core.ignition_priority_chains);
		for(auto& event : { &core.point_event , &core.instant_event }) {
			for(auto evit = event->begin(); evit != event->end(); ++evit) {
				auto pr = effects.activate_effect.equal_range(evit->event_code);
				for(auto eit = pr.first; eit != pr.second;) {
					effect* peffect = eit->second;
					++eit;
					peffect->set_activate_location();
					if(!peffect->is_flag(EFFECT_FLAG_DELAY) && peffect->is_chainable(priority) && peffect->is_activateable(priority, *evit)) {
						card* phandler = peffect->get_handler();
						core.select_chains.emplace_back();
						chain& newchain = core.select_chains.back();
						newchain.flag = 0;
						newchain.chain_id = infos.field_id++;
						newchain.evt = *evit;
						newchain.triggering_effect = peffect;
						newchain.set_triggering_state(phandler);
						newchain.triggering_player = priority;
					}
				}
				pr = effects.quick_o_effect.equal_range(evit->event_code);
				for(auto eit = pr.first; eit != pr.second;) {
					effect* peffect = eit->second;
					++eit;
					peffect->set_activate_location();
					if(!peffect->is_flag(EFFECT_FLAG_DELAY) && peffect->is_chainable(priority) && peffect->is_activateable(priority, *evit)) {
						card* phandler = peffect->get_handler();
						core.select_chains.emplace_back();
						chain& newchain = core.select_chains.back();
						newchain.flag = 0;
						newchain.chain_id = infos.field_id++;
						newchain.evt = *evit;
						newchain.triggering_effect = peffect;
						newchain.set_triggering_state(phandler);
						newchain.triggering_player = priority;
					}
				}
			}
		}
		for(auto& ch : core.new_ochain_h) {
			effect* peffect = ch.triggering_effect;
			card* phandler = peffect->get_handler();
			if(!peffect->is_flag(EFFECT_FLAG_FIELD_ONLY) && (peffect->type & EFFECT_TYPE_FIELD)
				&& (peffect->range & LOCATION_HAND) && phandler->current.location == LOCATION_HAND) {
				if(!phandler->is_has_relation(ch) && peffect->is_condition_check(phandler->current.controler, ch.evt))
					phandler->create_relation(ch);
				peffect->set_activate_location();
				ch.triggering_player = phandler->current.controler;
				ch.set_triggering_state(phandler);
			}
			if(ch.triggering_player == priority && !phandler->is_status(STATUS_CHAINING)
				&& ((ch.triggering_location == LOCATION_HAND && phandler->is_position(POS_FACEDOWN)) || ch.triggering_location == LOCATION_DECK)
				&& phandler->is_has_relation(ch) && peffect->is_chainable(priority) && peffect->is_activateable(priority, ch.evt, TRUE)
				&& check_spself_from_hand_trigger(ch))
				core.select_chains.push_back(ch);
		}
		//delayed activate
		for(const auto& ev : core.full_event) {
			auto pr = effects.activate_effect.equal_range(ev.event_code);
			for(auto eit = pr.first; eit != pr.second;) {
				effect* peffect = eit->second;
				++eit;
				peffect->set_activate_location();
				if(peffect->is_flag(EFFECT_FLAG_DELAY) && peffect->is_chainable(priority) && peffect->is_activateable(priority, ev)) {
					card* phandler = peffect->get_handler();
					core.select_chains.emplace_back();
					chain& newchain = core.select_chains.back();
					newchain.flag = 0;
					newchain.chain_id = infos.field_id++;
					newchain.evt = ev;
					newchain.triggering_effect = peffect;
					newchain.set_triggering_state(phandler);
					newchain.triggering_player = priority;
				}
			}
		}
		// delayed quick
		for(auto eit = core.delayed_quick.begin(); eit != core.delayed_quick.end();) {
			effect* peffect = eit->first;
			const tevent& evt = eit->second;
			++eit;
			peffect->set_activate_location();
			if(peffect->is_chainable(priority) && peffect->is_activateable(priority, evt, TRUE, FALSE, FALSE)) {
				card* phandler = peffect->get_handler();
				core.select_chains.emplace_back();
				chain& newchain = core.select_chains.back();
				newchain.flag = 0;
				newchain.chain_id = infos.field_id++;
				newchain.evt = evt;
				newchain.triggering_effect = peffect;
				newchain.set_triggering_state(phandler);
				newchain.triggering_player = priority;
			}
		}
		core.spe_effect[priority] = static_cast<int32_t>(core.select_chains.size());
		if(!skip_freechain) {
			nil_event.event_code = EVENT_FREE_CHAIN;
			auto pr = effects.activate_effect.equal_range(EVENT_FREE_CHAIN);
			for(auto eit = pr.first; eit != pr.second;) {
				effect* peffect = eit->second;
				++eit;
				peffect->set_activate_location();
				if(peffect->is_chainable(priority) && peffect->is_activateable(priority, nil_event)) {
					card* phandler = peffect->get_handler();
					core.select_chains.emplace_back();
					chain& newchain = core.select_chains.back();
					newchain.flag = 0;
					newchain.chain_id = infos.field_id++;
					newchain.evt = nil_event;
					newchain.triggering_effect = peffect;
					newchain.set_triggering_state(phandler);
					newchain.triggering_player = priority;
					if(check_hint_timing(peffect) || check_cteffect_hint(peffect, priority))
						++core.spe_effect[priority];
				}
			}
			pr = effects.quick_o_effect.equal_range(EVENT_FREE_CHAIN);
			for(auto eit = pr.first; eit != pr.second;) {
				effect* peffect = eit->second;
				++eit;
				peffect->set_activate_location();
				if(peffect->is_chainable(priority) && peffect->is_activateable(priority, nil_event)) {
					card* phandler = peffect->get_handler();
					core.select_chains.emplace_back();
					chain& newchain = core.select_chains.back();
					newchain.flag = 0;
					newchain.chain_id = infos.field_id++;
					newchain.evt = nil_event;
					newchain.triggering_effect = peffect;
					newchain.set_triggering_state(phandler);
					newchain.triggering_player = priority;
					if(check_hint_timing(peffect))
						++core.spe_effect[priority];
				}
			}
		}
		if(core.current_chain.size() || (core.hint_timing[0] & TIMING_ATTACK) || (core.hint_timing[1] & TIMING_ATTACK))
			core.spe_effect[priority] = static_cast<int32_t>(core.select_chains.size());
		add_process(PROCESSOR_SELECT_CHAIN, 0, 0, 0, priority, core.spe_effect[priority]);
		return FALSE;
	}
	case 3: {
		if(core.select_chains.size() && returns.at<int32_t>(0) != -1) {
			chain& newchain = core.select_chains[returns.at<int32_t>(0)];
			effect* peffect = newchain.triggering_effect;
			core.delayed_quick.erase(std::make_pair(peffect, newchain.evt));
			core.new_chains.push_back(std::move(newchain));
			peffect->get_handler()->set_status(STATUS_CHAINING, TRUE);
			peffect->dec_count(priority);
			add_process(PROCESSOR_ADD_CHAIN, 0, 0, 0, 0, 0);
			add_process(PROCESSOR_QUICK_EFFECT, 0, 0, 0, FALSE, 1 - priority);
			infos.priorities[0] = 0;
			infos.priorities[1] = 0;
		} else {
			infos.priorities[priority] = 1;
			if(!infos.priorities[0] || !infos.priorities[1])
				add_process(PROCESSOR_QUICK_EFFECT, 1, 0, 0, skip_freechain, 1 - priority);
			else {
				core.hint_timing[0] &= TIMING_DAMAGE_STEP | TIMING_DAMAGE_CAL;
				core.hint_timing[1] &= TIMING_DAMAGE_STEP | TIMING_DAMAGE_CAL;
			}
		}
		core.select_chains.clear();
		/*if(core.set_forced_attack) {
			core.set_forced_attack = false;
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);
		}*/
		return TRUE;
	}
	}
	/*if(core.set_forced_attack) {
		core.set_forced_attack = false;
		add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);
	}*/
	return TRUE;
}
int32_t field::process_instant_event() {
	auto check_simul = [&just_sent=core.just_sent_cards](effect* peffect, card* phandler) {
		return (peffect->flag[1] & EFFECT_FLAG2_CHECK_SIMULTANEOUS) != 0 && just_sent.find(phandler) != just_sent.end();
	};
	if(core.queue_event.size() == 0) {
		/*if(core.set_forced_attack)
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
		return TRUE;
	}
	chain_list tp;
	chain_list ntp;
	for(const auto& ev : core.queue_event) {
		//continuous events
		auto pr = effects.continuous_effect.equal_range(ev.event_code);
		for(auto eit = pr.first; eit != pr.second;) {
			effect* peffect = eit->second;
			++eit;
			uint8_t owner_player = peffect->get_handler_player();
			if(peffect->is_flag(EFFECT_FLAG_EVENT_PLAYER) && (ev.event_player == 0 || ev.event_player == 1))
				owner_player = ev.event_player;
			if(peffect->is_activateable(owner_player, ev)) {
				auto& chain_set = [&]() -> chain_list& {
					if(peffect->is_flag(EFFECT_FLAG_DELAY) && (core.chain_solving || core.conti_solving)) {
						if(owner_player == infos.turn_player)
							return core.delayed_continuous_tp;
						return core.delayed_continuous_ntp;
					}
					if(owner_player == infos.turn_player)
						return tp;
					return ntp;
				}();
				chain_set.emplace_back();
				chain& newchain = chain_set.back();
				newchain.chain_id = 0;
				newchain.chain_count = 0;
				newchain.triggering_effect = peffect;
				newchain.triggering_player = owner_player;
				newchain.evt = ev;
				newchain.target_cards = 0;
				newchain.target_player = PLAYER_NONE;
				newchain.target_param = 0;
				newchain.disable_player = PLAYER_NONE;
				newchain.disable_reason = 0;
				newchain.flag = 0;
			}
		}
		if(ev.event_code == EVENT_ADJUST || ev.event_code == EVENT_BREAK_EFFECT || ((ev.event_code & 0xf000) == EVENT_PHASE_START))
			continue;
		//triggers
		pr = effects.trigger_f_effect.equal_range(ev.event_code);
		for(auto eit = pr.first; eit != pr.second;) {
			effect* peffect = eit->second;
			++eit;
			card* phandler = peffect->get_handler();
			if(!phandler->is_status(STATUS_EFFECT_ENABLED) || !peffect->is_condition_check(phandler->current.controler, ev))
				continue;
			bool was_just_sent = false;
			if((was_just_sent = check_simul(peffect, phandler)) == true && (peffect->range & LOCATION_HAND) == 0)
				continue;
			peffect->set_activate_location();
			core.new_fchain.emplace_back();
			chain& newchain = core.new_fchain.back();
			newchain.was_just_sent = was_just_sent;
			newchain.flag = 0;
			newchain.chain_id = infos.field_id++;
			newchain.event_id = ev.global_id;
			newchain.evt = ev;
			newchain.triggering_effect = peffect;
			newchain.set_triggering_state(phandler);
			if(peffect->is_flag(EFFECT_FLAG_EVENT_PLAYER) && (ev.event_player == 0 || ev.event_player == 1))
				newchain.triggering_player = ev.event_player;
			else
				newchain.triggering_player = phandler->current.controler;
			phandler->create_relation(newchain);
		}
		pr = effects.trigger_o_effect.equal_range(ev.event_code);
		for(auto eit = pr.first; eit != pr.second;) {
			effect* peffect = eit->second;
			++eit;
			card* phandler = peffect->get_handler();
			bool act = phandler->is_status(STATUS_EFFECT_ENABLED) && peffect->is_condition_check(phandler->current.controler, ev);
			bool was_just_sent = false;
			if((was_just_sent = check_simul(peffect, phandler)) == true && (peffect->range & LOCATION_HAND) == 0)
				continue;
			if((peffect->range & LOCATION_HAND) == 0 && !act)
				continue;
			peffect->set_activate_location();
			core.new_ochain.emplace_back();
			chain& newchain = core.new_ochain.back();
			newchain.was_just_sent = was_just_sent;
			newchain.flag = 0;
			newchain.chain_id = infos.field_id++;
			newchain.event_id = ev.global_id;
			newchain.evt = ev;
			newchain.triggering_effect = peffect;
			newchain.set_triggering_state(phandler);
			if(peffect->is_flag(EFFECT_FLAG_EVENT_PLAYER) && (ev.event_player == 0 || ev.event_player == 1))
				newchain.triggering_player = ev.event_player;
			else
				newchain.triggering_player = phandler->current.controler;
			if(peffect->is_flag(EFFECT_FLAG_FIELD_ONLY)
				|| !(peffect->range & LOCATION_HAND)
				|| ((peffect->range & phandler->current.location) && act))
				phandler->create_relation(newchain);
		}
		//instant_f
		pr = effects.quick_f_effect.equal_range(ev.event_code);
		for(auto eit = pr.first; eit != pr.second;) {
			effect* peffect = eit->second;
			++eit;
			card* phandler = peffect->get_handler();
			peffect->set_activate_location();
			if(peffect->is_activateable(phandler->current.controler, ev)) {
				chain& newchain = core.quick_f_chain[peffect];
				newchain.flag = 0;
				newchain.chain_id = infos.field_id++;
				newchain.evt = ev;
				newchain.triggering_effect = peffect;
				newchain.set_triggering_state(phandler);
				if(peffect->is_flag(EFFECT_FLAG_EVENT_PLAYER) && (ev.event_player == 0 || ev.event_player == 1))
					newchain.triggering_player = ev.event_player;
				else
					newchain.triggering_player = phandler->current.controler;
				phandler->create_relation(newchain);
			}
		}
		// delayed activate effect
		core.delayed_activate_event.push_back(ev);
		// delayed quick effect
		pr = effects.quick_o_effect.equal_range(ev.event_code);
		for(auto eit = pr.first; eit != pr.second;) {
			effect* peffect = eit->second;
			++eit;
			if(peffect->is_flag(EFFECT_FLAG_DELAY) && peffect->is_condition_check(peffect->get_handler()->current.controler, ev))
				core.delayed_quick_tmp.emplace(peffect, ev);
		}
	}
	while(tp.size()) {
		core.sub_solving_continuous.splice(core.sub_solving_continuous.end(), tp, tp.begin());
		add_process(PROCESSOR_SOLVE_CONTINUOUS, 0, 0, 0, 0, 0);
	}
	while(ntp.size()) {
		core.sub_solving_continuous.splice(core.sub_solving_continuous.end(), ntp, ntp.begin());
		add_process(PROCESSOR_SOLVE_CONTINUOUS, 0, 0, 0, 0, 0);
	}
	core.instant_event.splice(core.instant_event.end(), core.queue_event);
	/*if(core.set_forced_attack) {
		core.set_forced_attack = false;
		add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);
	}*/
	return TRUE;
}
int32_t field::process_single_event() {
	if(core.single_event.size() == 0) {
		/*if(core.set_forced_attack) {
			core.set_forced_attack = false;
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);
		}*/
		return TRUE;
	}
	chain_list tp;
	chain_list ntp;
	for(const auto& ev : core.single_event) {
		card* starget = ev.trigger_card;
		auto pr = starget->single_effect.equal_range(ev.event_code);
		for(auto eit = pr.first; eit != pr.second;) {
			effect* peffect = eit->second;
			++eit;
			process_single_event(peffect, ev, tp, ntp);
		}
		for(auto& pcard : starget->xyz_materials) {
			pr = pcard->xmaterial_effect.equal_range(ev.event_code);
			for(auto eit = pr.first; eit != pr.second;) {
				effect* peffect = eit->second;
				++eit;
				if(peffect->type & EFFECT_TYPE_FIELD)
					continue;
				process_single_event(peffect, ev, tp, ntp);
			}
		}
	}
	while(tp.size()) {
		core.sub_solving_continuous.splice(core.sub_solving_continuous.end(), tp, tp.begin());
		add_process(PROCESSOR_SOLVE_CONTINUOUS, 0, 0, 0, 0, 0);
	}
	while(ntp.size()) {
		core.sub_solving_continuous.splice(core.sub_solving_continuous.end(), ntp, ntp.begin());
		add_process(PROCESSOR_SOLVE_CONTINUOUS, 0, 0, 0, 0, 0);
	}
	core.single_event.clear();
	/*if(core.set_forced_attack) {
		core.set_forced_attack = false;
		add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);
	}*/
	return TRUE;
}
int32_t field::process_single_event(effect* peffect, const tevent& e, chain_list& tp, chain_list& ntp) {
	if(!(peffect->type & EFFECT_TYPE_ACTIONS))
		return FALSE;
	if((peffect->type & EFFECT_TYPE_FLIP) && (e.event_value & (NO_FLIP_EFFECT >> 16)))
		return FALSE;
	//continuous & trigger (single)
	if(peffect->type & EFFECT_TYPE_CONTINUOUS) {
		uint8_t owner_player = peffect->get_handler_player();
		if(peffect->is_flag(EFFECT_FLAG_EVENT_PLAYER) && (e.event_player == 0 || e.event_player == 1))
			owner_player = e.event_player;
		if(peffect->is_activateable(owner_player, e)) {
			auto& chain_set = [&]() -> chain_list& {
				if(peffect->is_flag(EFFECT_FLAG_DELAY) && (core.chain_solving || core.conti_solving)) {
					if(owner_player == infos.turn_player)
						return core.delayed_continuous_tp;
					return core.delayed_continuous_ntp;
				}
				if(owner_player == infos.turn_player)
					return tp;
				return ntp;
			}();
			chain_set.emplace_back();
			chain& newchain = chain_set.back();
			newchain.chain_id = 0;
			newchain.chain_count = 0;
			newchain.triggering_effect = peffect;
			newchain.triggering_player = owner_player;
			newchain.evt = e;
			newchain.target_cards = 0;
			newchain.target_player = PLAYER_NONE;
			newchain.target_param = 0;
			newchain.disable_player = PLAYER_NONE;
			newchain.disable_reason = 0;
			newchain.flag = 0;
		}
	} else {
		card* phandler = peffect->get_handler();
		if(!peffect->is_condition_check(phandler->current.controler, e))
			return FALSE;
		peffect->set_activate_location();
		auto& chain_set = [&]() -> chain_list& {
			if(core.flip_delayed && e.event_code == EVENT_FLIP) {
				if(peffect->type & EFFECT_TYPE_TRIGGER_O)
					return core.new_ochain_b;
				return core.new_fchain_b;
			}
			if(peffect->type & EFFECT_TYPE_TRIGGER_O)
				return core.new_ochain;
			return core.new_fchain;
		}();
		chain_set.emplace_back();
		chain& newchain = chain_set.back();
		newchain.flag = 0;
		newchain.chain_id = infos.field_id++;
		newchain.event_id = e.global_id;
		newchain.evt = e;
		newchain.triggering_effect = peffect;
		newchain.set_triggering_state(phandler);
		if(peffect->is_flag(EFFECT_FLAG_EVENT_PLAYER) && (e.event_player == 0 || e.event_player == 1))
			newchain.triggering_player = e.event_player;
		else {
			if(phandler->current.reason & REASON_TEMPORARY)
				newchain.triggering_player = phandler->previous.controler;
			else
				newchain.triggering_player = newchain.triggering_controler;
		}
		peffect->set_active_type();
		phandler->create_relation(newchain);
		effect* deffect;
		if((deffect = phandler->is_affected_by_effect(EFFECT_DISABLE_EFFECT)) != nullptr) {
			effect* negeff = pduel->new_effect();
			negeff->owner = deffect->owner;
			negeff->type = EFFECT_TYPE_SINGLE;
			negeff->code = EFFECT_DISABLE_CHAIN;
			negeff->value = newchain.chain_id;
			negeff->reset_flag = RESET_EVENT | deffect->get_value();
			phandler->add_effect(negeff);
		}
	}
	/*if(core.set_forced_attack) {
		core.set_forced_attack = false;
		add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);
	}*/
	return TRUE;
}
int32_t field::process_idle_command(uint16_t step) {
	switch(step) {
	case 0: {
		bool must_attack = false;
		core.select_chains.clear();
		nil_event.event_code = EVENT_FREE_CHAIN;
		if(core.set_forced_attack) {
			core.set_forced_attack = false;
			core.units.begin()->step = -1;
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);
			return FALSE;
		}
		core.to_bp = TRUE;
		core.to_ep = TRUE;
		if((!is_flag(DUEL_ATTACK_FIRST_TURN) && infos.turn_id == 1 && !(is_player_affected_by_effect(infos.turn_player, EFFECT_BP_FIRST_TURN))) || infos.phase == PHASE_MAIN2 || is_player_affected_by_effect(infos.turn_player, EFFECT_CANNOT_BP) || core.force_turn_end)
			core.to_bp = FALSE;
		if(infos.phase == PHASE_MAIN1) {
			for(auto& pcard : player[infos.turn_player].list_mzone) {
				if(pcard && pcard->is_capable_attack() && pcard->is_affected_by_effect(EFFECT_MUST_ATTACK)) {
					must_attack = true;
					break;
				}
			}
			if(core.to_bp && (must_attack || is_player_affected_by_effect(infos.turn_player, EFFECT_CANNOT_EP)))
				core.to_ep = FALSE;
		}
		if((infos.phase == PHASE_MAIN1 && is_player_affected_by_effect(infos.turn_player, EFFECT_SKIP_M1))
		        || (infos.phase == PHASE_MAIN2 && is_player_affected_by_effect(infos.turn_player, EFFECT_SKIP_M2)) || core.force_turn_end) {
			if(core.to_bp && core.to_ep) {
				core.select_options.clear();
				core.select_options.push_back(80);
				core.select_options.push_back(81);
				add_process(PROCESSOR_SELECT_OPTION, 0, 0, 0, infos.turn_player, 0);
				core.units.begin()->step = 11;
			} else if(core.to_bp) {
				core.units.begin()->arg1 = 6;
				core.units.begin()->step = 10;
				reset_phase(infos.phase);
				adjust_all();
			} else {
				core.units.begin()->arg1 = 7;
				core.units.begin()->step = 10;
				reset_phase(infos.phase);
				adjust_all();
			}
			return FALSE;
		}
		if((infos.phase == PHASE_MAIN2) && core.skip_m2) {
			core.skip_m2 = FALSE;
			returns.set<int32_t>(0, 7);
			return FALSE;
		}
		auto pr = effects.activate_effect.equal_range(EVENT_FREE_CHAIN);
		for(auto eit = pr.first; eit != pr.second; eit++) {
			effect* peffect = eit->second;
			peffect->set_activate_location();
			if(peffect->is_activateable(infos.turn_player, nil_event)) {
				core.select_chains.emplace_back();
				core.select_chains.back().triggering_effect = peffect;
			}
		}
		pr = effects.quick_o_effect.equal_range(EVENT_FREE_CHAIN);
		for(auto eit = pr.first; eit != pr.second; eit++) {
			effect* peffect = eit->second;
			peffect->set_activate_location();
			if(peffect->is_activateable(infos.turn_player, nil_event)) {
				core.select_chains.emplace_back();
				core.select_chains.back().triggering_effect = peffect;
			}
		}
		pr = effects.continuous_effect.equal_range(EVENT_FREE_CHAIN);
		for(auto eit = pr.first; eit != pr.second; eit++) {
			effect* peffect = eit->second;
			if(peffect->get_handler_player() == infos.turn_player && peffect->is_activateable(infos.turn_player, nil_event)) {
				core.select_chains.emplace_back();
				core.select_chains.back().triggering_effect = peffect;
			}
		}
		for(auto& eit : effects.ignition_effect) {
			effect* peffect = eit.second;
			peffect->set_activate_location();
			if(peffect->is_activateable(infos.turn_player, nil_event)) {
				core.select_chains.emplace_back();
				core.select_chains.back().triggering_effect = peffect;
			}
		}
		core.summonable_cards.clear();
		for(auto& pcard : player[infos.turn_player].list_hand)
			if(pcard->is_can_be_summoned(infos.turn_player, FALSE, 0, 0))
				core.summonable_cards.push_back(pcard);
		for(auto& pcard : player[infos.turn_player].list_mzone) {
			if(pcard && pcard->is_can_be_summoned(infos.turn_player, FALSE, 0, 0))
				core.summonable_cards.push_back(pcard);
		}
		core.spsummonable_cards.clear();
		effect_set eset;
		filter_field_effect(EFFECT_SPSUMMON_PROC, &eset);
		for(const auto& peff : eset) {
			card* pcard = peff->get_handler();
			if(!peff->check_count_limit(pcard->current.controler))
				continue;
			if(pcard->current.controler == infos.turn_player && pcard->is_special_summonable(infos.turn_player, 0))
				core.spsummonable_cards.push_back(pcard);
		}
		eset.clear();
		filter_field_effect(EFFECT_SPSUMMON_PROC_G, &eset);
		for(const auto& peff : eset) {
			card* pcard = peff->get_handler();
			if(!peff->check_count_limit(infos.turn_player))
				continue;
			if(pcard->current.controler != infos.turn_player && !peff->is_flag(EFFECT_FLAG_BOTH_SIDE))
				continue;
			effect* oreason = core.reason_effect;
			uint8_t op = core.reason_player;
			core.reason_effect = peff;
			core.reason_player = pcard->current.controler;
			save_lp_cost();
			pduel->lua->add_param<PARAM_TYPE_EFFECT>(peff);
			pduel->lua->add_param<PARAM_TYPE_CARD>(pcard);
			if(pduel->lua->check_condition(peff->condition, 2))
				core.spsummonable_cards.push_back(pcard);
			restore_lp_cost();
			core.reason_effect = oreason;
			core.reason_player = op;
		}
		core.repositionable_cards.clear();
		for(auto& pcard : player[infos.turn_player].list_mzone) {
			if(pcard && ((pcard->is_position(POS_FACEUP | POS_FACEDOWN_ATTACK) && pcard->is_capable_change_position(infos.turn_player))
		        || (pcard->is_position(POS_FACEDOWN) && pcard->is_can_be_flip_summoned(infos.turn_player))))
				core.repositionable_cards.push_back(pcard);
		}
		core.msetable_cards.clear();
		core.ssetable_cards.clear();
		for(auto& pcard : player[infos.turn_player].list_hand) {
			if(pcard->is_setable_mzone(infos.turn_player, FALSE, 0, 0))
				core.msetable_cards.push_back(pcard);
			if(pcard->is_setable_szone(infos.turn_player))
				core.ssetable_cards.push_back(pcard);
		}
		add_process(PROCESSOR_SELECT_IDLECMD, 0, 0, 0, infos.turn_player, 0);
		return FALSE;
	}
	case 1: {
		uint32_t ctype = returns.at<int32_t>(0) & 0xffff;
		uint32_t sel = returns.at<int32_t>(0) >> 16;
		if(ctype == 5) {
			chain& newchain = core.select_chains[sel];
			effect* peffect = newchain.triggering_effect;
			if(peffect->type & EFFECT_TYPE_CONTINUOUS) {
				core.select_chains.clear();
				solve_continuous(peffect->get_handler_player(), peffect, nil_event);
				core.units.begin()->step = 2;
				return FALSE;
			}
			card* phandler = peffect->get_handler();
			newchain.flag = 0;
			newchain.chain_id = infos.field_id++;
			newchain.evt.event_code = peffect->code;
			newchain.evt.event_player = PLAYER_NONE;
			newchain.evt.event_value = 0;
			newchain.evt.event_cards = 0;
			newchain.evt.reason = 0;
			newchain.evt.reason_effect = 0;
			newchain.evt.reason_player = PLAYER_NONE;
			newchain.set_triggering_state(phandler);
			newchain.triggering_player = infos.turn_player;
			core.new_chains.push_back(std::move(newchain));
			phandler->set_status(STATUS_CHAINING, TRUE);
			peffect->dec_count(infos.turn_player);
			core.select_chains.clear();
			add_process(PROCESSOR_ADD_CHAIN, 0, 0, 0, 0, 0);
			add_process(PROCESSOR_QUICK_EFFECT, 0, 0, 0, FALSE, 1 - infos.turn_player);
			infos.priorities[0] = 0;
			infos.priorities[1] = 0;
			core.select_chains.clear();
			return FALSE;
		} else if(ctype == 0) {
			core.units.begin()->step = 4;
			return FALSE;
		} else if(ctype == 1) {
			core.units.begin()->step = 5;
			return FALSE;
		} else if(ctype == 2) {
			core.units.begin()->step = 6;
			return FALSE;
		} else if(ctype == 3) {
			core.units.begin()->step = 7;
			return FALSE;
		} else if(ctype == 4) {
			core.units.begin()->step = 8;
			return FALSE;
		} else if (ctype == 8) {
			core.units.begin()->step = -1;
			shuffle(infos.turn_player, LOCATION_HAND);
			infos.can_shuffle = FALSE;
			return FALSE;
		} else {
			core.units.begin()->step = 9;
			auto message = pduel->new_message(MSG_HINT);
			message->write<uint8_t>(HINT_EVENT);
			message->write<uint8_t>(1 - infos.turn_player);
			message->write<uint64_t>(23);
			core.select_chains.clear();
			core.hint_timing[infos.turn_player] = TIMING_MAIN_END;
			add_process(PROCESSOR_QUICK_EFFECT, 0, 0, 0, FALSE, 1 - infos.turn_player);
			infos.priorities[infos.turn_player] = 1;
			infos.priorities[1 - infos.turn_player] = 0;
			core.units.begin()->arg1 = ctype;
			return FALSE;
		}
		/*if(core.set_forced_attack) {
			core.set_forced_attack = false;
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);
		}*/
		return TRUE;
	}
	case 2: {
		for(auto& ch_lim : core.chain_limit)
			luaL_unref(pduel->lua->lua_state, LUA_REGISTRYINDEX, ch_lim.function);
		core.chain_limit.clear();
		for(auto& ch : core.current_chain)
			ch.triggering_effect->get_handler()->set_status(STATUS_CHAINING, FALSE);
		add_process(PROCESSOR_SOLVE_CHAIN, 0, 0, 0, FALSE, 0);
		core.units.begin()->step = -1;
		return FALSE;
	}
	case 3: {
		adjust_instant();
		add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, 0, 0);
		core.units.begin()->step = -1;
		return FALSE;
	}
	case 5: {
		card* target = core.summonable_cards[returns.at<int32_t>(0) >> 16];
		core.summon_cancelable = TRUE;
		summon(infos.turn_player, target, 0, FALSE, 0);
		core.units.begin()->step = -1;
		return FALSE;
	}
	case 6: {
		card* target = core.spsummonable_cards[returns.at<int32_t>(0) >> 16];
		core.summon_cancelable = TRUE;
		special_summon_rule(infos.turn_player, target, 0);
		core.units.begin()->step = -1;
		return FALSE;
	}
	case 7: {
		card* target = core.repositionable_cards[returns.at<int32_t>(0) >> 16];
		if(target->is_position(POS_FACEUP_ATTACK)) {
			core.phase_action = TRUE;
			change_position(target, 0, infos.turn_player, POS_FACEUP_DEFENSE, FALSE);
			adjust_all();
			add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, FALSE, 0);
		} else if(target->is_position(POS_FACEUP_DEFENSE)) {
			core.phase_action = TRUE;
			change_position(target, 0, infos.turn_player, POS_FACEUP_ATTACK, FALSE);
			adjust_all();
			add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, FALSE, 0);
		} else if(target->is_position(POS_FACEDOWN_ATTACK)) {
			core.units.begin()->ptarget = (group*)target;
			int32_t positions = 0;
			if(target->is_capable_change_position(infos.turn_player))
				positions |= POS_FACEDOWN_DEFENSE;
			if(target->is_can_be_flip_summoned(infos.turn_player))
				positions |= POS_FACEUP_ATTACK;
			add_process(PROCESSOR_SELECT_POSITION, 0, 0, 0, infos.turn_player + (positions << 16), target->data.code);
			core.units.begin()->step = 12;
			return FALSE;
		} else
			add_process(PROCESSOR_FLIP_SUMMON, 0, 0, (group*)target, target->current.controler, 0);
		target->set_status(STATUS_FORM_CHANGED, TRUE);
		core.units.begin()->step = -1;
		return FALSE;
	}
	case 8: {
		card* target = core.msetable_cards[returns.at<int32_t>(0) >> 16];
		core.summon_cancelable = TRUE;
		mset(target->current.controler, target, 0, FALSE, 0);
		core.units.begin()->step = -1;
		return FALSE;
	}
	case 9: {
		card* target = core.ssetable_cards[returns.at<int32_t>(0) >> 16];
		add_process(PROCESSOR_SSET, 0, 0, (group*)target, target->current.controler, target->current.controler);
		core.units.begin()->step = -1;
		return FALSE;
	}
	case 10: {
		//end announce
		for(auto& ch_lim : core.chain_limit)
			luaL_unref(pduel->lua->lua_state, LUA_REGISTRYINDEX, ch_lim.function);
		core.chain_limit.clear();
		if(core.current_chain.size()) {
			for(auto& ch : core.current_chain)
				ch.triggering_effect->get_handler()->set_status(STATUS_CHAINING, FALSE);
			add_process(PROCESSOR_SOLVE_CHAIN, 0, 0, 0, FALSE, 0);
			core.units.begin()->step = -1;
			return FALSE;
		}
		reset_phase(infos.phase);
		adjust_all();
		return FALSE;
	}
	case 11: {
		returns.set<int32_t>(0, core.units.begin()->arg1);
		infos.can_shuffle = TRUE;
		/*if(core.set_forced_attack) {
			core.set_forced_attack = false;
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);
		}*/
		return TRUE;
	}
	case 12: {
		if(returns.at<int32_t>(0) == 0)
			core.units.begin()->arg1 = 6;
		else
			core.units.begin()->arg1 = 7;
		reset_phase(infos.phase);
		adjust_all();
		core.units.begin()->step = 10;
		return FALSE;
	}
	case 13: {
		card* target = (card*)core.units.begin()->ptarget;
		if(returns.at<int32_t>(0) == POS_FACEUP_ATTACK)
			add_process(PROCESSOR_FLIP_SUMMON, 0, 0, (group*)target, target->current.controler, 0);
		else {
			core.phase_action = TRUE;
			change_position(target, 0, infos.turn_player, POS_FACEDOWN_DEFENSE, FALSE);
			adjust_all();
			add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, FALSE, 0);
		}
		target->set_status(STATUS_FORM_CHANGED, TRUE);
		core.units.begin()->step = -1;
		return FALSE;
	}
	}
	/*if(core.set_forced_attack) {
		core.set_forced_attack = false;
		add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);
	}*/
	return TRUE;
}
int32_t field::process_battle_command(uint16_t step) {
	switch(step) {
	case 0: {
		effect* peffect = 0;
		core.select_chains.clear();
		nil_event.event_code = EVENT_FREE_CHAIN;
		if(!core.chain_attack) {
			core.chain_attacker_id = 0;
			core.chain_attack_target = 0;
		}
		core.attack_player = FALSE;
		core.attacker = 0;
		core.attack_target = 0;
		if((peffect = is_player_affected_by_effect(infos.turn_player, EFFECT_SKIP_BP)) != nullptr || core.force_turn_end) {
			core.units.begin()->step = 41;
			core.units.begin()->arg1 = 2;
			if(is_player_affected_by_effect(infos.turn_player, EFFECT_BP_TWICE))
				core.units.begin()->arg2 = 1;
			else core.units.begin()->arg2 = 0;
			if(core.force_turn_end || !peffect->value) {
				reset_phase(PHASE_BATTLE_STEP);
				adjust_all();
				infos.phase = PHASE_BATTLE;
				add_process(PROCESSOR_PHASE_EVENT, 0, 0, 0, PHASE_BATTLE, 0);
			} else {
				core.hint_timing[infos.turn_player] = 0;
				reset_phase(PHASE_BATTLE);
				adjust_all();
			}
			return FALSE;
		}
		auto pr = effects.activate_effect.equal_range(EVENT_FREE_CHAIN);
		for(auto eit = pr.first; eit != pr.second; eit++) {
			peffect = eit->second;
			peffect->set_activate_location();
			if(peffect->is_activateable(infos.turn_player, nil_event) && peffect->get_speed() > 1) {
				core.select_chains.emplace_back();
				core.select_chains.back().triggering_effect = peffect;
			}
		}
		pr = effects.quick_o_effect.equal_range(EVENT_FREE_CHAIN);
		for(auto eit = pr.first; eit != pr.second; eit++) {
			peffect = eit->second;
			peffect->set_activate_location();
			if(peffect->is_activateable(infos.turn_player, nil_event)) {
				core.select_chains.emplace_back();
				core.select_chains.back().triggering_effect = peffect;
			}
		}
		pr = effects.continuous_effect.equal_range(EVENT_FREE_CHAIN);
		for(auto eit = pr.first; eit != pr.second; eit++) {
			peffect = eit->second;
			if(peffect->get_handler_player() == infos.turn_player && peffect->is_activateable(infos.turn_player, nil_event)) {
				core.select_chains.emplace_back();
				core.select_chains.back().triggering_effect = peffect;
			}
		}
		core.attackable_cards.clear();
		card_vector first_attack;
		card_vector must_attack;
		if(!is_player_affected_by_effect(infos.turn_player, EFFECT_CANNOT_ATTACK_ANNOUNCE)) {
			for(auto& pcard : player[infos.turn_player].list_mzone) {
				if(!pcard)
					continue;
				if(!pcard->is_capable_attack_announce(infos.turn_player))
					continue;
				uint8_t chain_attack = FALSE;
				if(core.chain_attack && core.chain_attacker_id == pcard->fieldid)
					chain_attack = TRUE;
				card_vector cv;
				get_attack_target(pcard, &cv, chain_attack);
				if(cv.size() == 0 && pcard->direct_attackable == 0)
					continue;
				core.attackable_cards.push_back(pcard);
				if(pcard->is_affected_by_effect(EFFECT_FIRST_ATTACK))
					first_attack.push_back(pcard);
				if(pcard->is_affected_by_effect(EFFECT_MUST_ATTACK))
					must_attack.push_back(pcard);
			}
			if(first_attack.size())
				core.attackable_cards = first_attack;
		}
		core.to_m2 = is_flag(DUEL_NO_MAIN_PHASE_2) ? FALSE : TRUE;
		core.to_ep = TRUE;
		if(must_attack.size() || is_player_affected_by_effect(infos.turn_player, EFFECT_CANNOT_M2) || core.force_turn_end)
			core.to_m2 = FALSE;
		if(must_attack.size())
			core.to_ep = FALSE;
		core.attack_cancelable = TRUE;
		core.attack_cost_paid = FALSE;
		add_process(PROCESSOR_SELECT_BATTLECMD, 0, 0, 0, infos.turn_player, 0);
		return FALSE;
	}
	case 1: {
		int32_t ctype = returns.at<int32_t>(0) & 0xffff;
		int32_t sel = returns.at<int32_t>(0) >> 16;
		if(ctype == 0) {
			chain& newchain = core.select_chains[sel];
			effect* peffect = newchain.triggering_effect;
			if(peffect->type & EFFECT_TYPE_CONTINUOUS) {
				core.select_chains.clear();
				solve_continuous(peffect->get_handler_player(), peffect, nil_event);
				core.units.begin()->step = 14;
				return FALSE;
			}
			card* phandler = peffect->get_handler();
			newchain.flag = 0;
			newchain.chain_id = infos.field_id++;
			newchain.evt.event_code = peffect->code;
			newchain.evt.event_player = PLAYER_NONE;
			newchain.evt.event_value = 0;
			newchain.evt.event_cards = 0;
			newchain.evt.reason = 0;
			newchain.evt.reason_effect = 0;
			newchain.evt.reason_player = PLAYER_NONE;
			newchain.set_triggering_state(phandler);
			newchain.triggering_player = infos.turn_player;
			core.new_chains.push_back(std::move(newchain));
			phandler->set_status(STATUS_CHAINING, TRUE);
			peffect->dec_count(infos.turn_player);
			core.select_chains.clear();
			add_process(PROCESSOR_ADD_CHAIN, 0, 0, 0, 0, 0);
			add_process(PROCESSOR_QUICK_EFFECT, 0, 0, 0, FALSE, 1 - infos.turn_player);
			infos.priorities[0] = 0;
			infos.priorities[1] = 0;
			core.select_chains.clear();
			return FALSE;
		} else if(ctype == 1) {
			core.units.begin()->step = 2;
			core.units.begin()->arg3 = FALSE;
			if(!core.forced_attack)
				core.attacker = core.attackable_cards[sel];
			core.attacker->set_status(STATUS_ATTACK_CANCELED, FALSE);
			core.attacker->attack_controler = core.attacker->current.controler;
			core.pre_field[0] = core.attacker->fieldid_r;
			if(core.chain_attack && core.chain_attacker_id != core.attacker->fieldid) {
				core.chain_attack = FALSE;
				core.chain_attacker_id = 0;
			}
			effect_set eset;
			core.tpchain.clear();
			filter_player_effect(infos.turn_player, EFFECT_ATTACK_COST, &eset, FALSE);
			core.attacker->filter_effect(EFFECT_ATTACK_COST, &eset);
			for(const auto& peff : eset) {
				if(peff->operation) {
					core.tpchain.emplace_back();
					core.tpchain.back().triggering_effect = peff;
					core.attack_cancelable = FALSE;
				}
			}
			if(core.tpchain.size() > 1) {
				add_process(PROCESSOR_SORT_CHAIN, 0, 0, 0, 1, infos.turn_player);
				core.units.begin()->step = 13;
			}
			else if(core.tpchain.size() == 1){
				core.sub_solving_event.push_back(nil_event);
				add_process(PROCESSOR_EXECUTE_OPERATION, 0, core.tpchain.begin()->triggering_effect, 0, infos.turn_player, 0);
				adjust_all();
			}
			return FALSE;
		} else {
			core.units.begin()->step = 39;
			core.units.begin()->arg1 = ctype;
			auto message = pduel->new_message(MSG_HINT);
			message->write<uint8_t>(HINT_EVENT);
			message->write<uint8_t>(1 - infos.turn_player);
			message->write<uint64_t>(29);
			core.select_chains.clear();
			core.hint_timing[infos.turn_player] = TIMING_BATTLE_STEP_END;
			add_process(PROCESSOR_QUICK_EFFECT, 0, 0, 0, FALSE, 1 - infos.turn_player);
			infos.priorities[infos.turn_player] = 1;
			infos.priorities[1 - infos.turn_player] = 0;
			return FALSE;
		}
		/*if(core.set_forced_attack) {
			core.set_forced_attack = false;
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);
		}*/
		return TRUE;
	}
	case 2: {
		for(auto& ch_lim : core.chain_limit)
			luaL_unref(pduel->lua->lua_state, LUA_REGISTRYINDEX, ch_lim.function);
		core.chain_limit.clear();
		for(auto& ch : core.current_chain)
			ch.triggering_effect->get_handler()->set_status(STATUS_CHAINING, FALSE);
		add_process(PROCESSOR_SOLVE_CHAIN, 0, 0, 0, FALSE, 0);
		core.units.begin()->step = -1;
		return FALSE;
	}
	case 3: {
		core.units.begin()->arg1 = FALSE;
		if(core.attacker->is_status(STATUS_ATTACK_CANCELED)) {
			core.units.begin()->arg3 = TRUE;
			core.units.begin()->step = 6;
			return FALSE;
		}
		//Cancel the attack cost
		if(core.attack_cost_paid != 1 && !core.attack_cancelable) {
			if(core.forced_attack) {
				returns.set<int32_t>(0, 2);
				core.units.begin()->step = 0;
			} else
				core.units.begin()->step = -1;
			return FALSE;
		}
		if(core.forced_attack)
			core.units.begin()->step = 6;
		return FALSE;
	}
	case 4: {
		// select attack target(replay start point)
		core.attack_player = FALSE;
		core.select_cards.clear();
		return_cards.clear();
		auto atype = get_attack_target(core.attacker, &core.select_cards, core.chain_attack);
		// direct attack
		if(core.attacker->direct_attackable) {
			if(core.select_cards.size() == 0) {
				returns.set<int32_t>(0, -2);
				core.units.begin()->step = 5;
				return FALSE;
			}
			if(is_player_affected_by_effect(infos.turn_player, EFFECT_PATRICIAN_OF_DARKNESS)) {
				add_process(PROCESSOR_SELECT_EFFECTYN, 0, 0, (group*)core.attacker, 1 - infos.turn_player, 31);
			}
			else {
				add_process(PROCESSOR_SELECT_YESNO, 0, 0, 0, infos.turn_player, 31);
			}
			return FALSE;
		}
		// no target and not direct attackable
		if(core.select_cards.size() == 0) {
			core.units.begin()->arg3 = TRUE;
			core.units.begin()->step = 6;
			return FALSE;
		}
		// must attack monster
		if(atype == 3 || is_player_affected_by_effect(infos.turn_player, EFFECT_PATRICIAN_OF_DARKNESS)) {
			if(core.select_cards.size() == 1)
				return_cards.list.push_back(core.select_cards.front());
			else {
				auto message = pduel->new_message(MSG_CARD_SELECTED);
				message->write<uint32_t>(1);
				message->write(core.attacker->get_info_location());
				message = pduel->new_message(MSG_HINT);
				message->write<uint8_t>(HINT_SELECTMSG);
				message->write<uint8_t>(1 - infos.turn_player);
				message->write<uint64_t>(549);
				add_process(PROCESSOR_SELECT_CARD, 0, 0, 0, 1 - infos.turn_player, 0x10001);
			}
		} else {
			auto message = pduel->new_message(MSG_HINT);
			message->write<uint8_t>(HINT_SELECTMSG);
			message->write<uint8_t>(infos.turn_player);
			message->write<uint64_t>(549);
			add_process(PROCESSOR_SELECT_CARD, 0, 0, 0, infos.turn_player + (core.attack_cancelable ? 0x20000 : 0), 0x10001);
		}
		core.units.begin()->step = 5;
		return FALSE;
	}
	case 5: {
		// the answer of "direct attack or not"
		if(returns.at<int32_t>(0)) {
			returns.set<int32_t>(0, -2);
		} else {
			if(core.select_cards.size()) {
				auto opposel = !!is_player_affected_by_effect(infos.turn_player, EFFECT_PATRICIAN_OF_DARKNESS);
				const auto sel_player = opposel ? 1 - infos.turn_player : infos.turn_player;
				const auto cancelable = (core.attack_cancelable && !opposel) ? 0x20000 : 0;
				auto message = pduel->new_message(MSG_HINT);
				message->write<uint8_t>(HINT_SELECTMSG);
				message->write<uint8_t>(opposel ? 1 - infos.turn_player : infos.turn_player);
				message->write<uint64_t>(549);
				add_process(PROCESSOR_SELECT_CARD, 0, 0, 0, sel_player + cancelable, 0x10001);
			} else {
				core.units.begin()->arg3 = TRUE;
				core.units.begin()->step = 6;
			}
		}
		return FALSE;
	}
	case 6: {
		if(return_cards.canceled) {//cancel attack manually
			if(core.units.begin()->arg1) {
				core.units.begin()->step = 12;
				return FALSE;
			}
			if(core.forced_attack) {
				returns.set<int32_t>(0, 2);
				core.units.begin()->step = 0;
			} else
				core.units.begin()->step = -1;
			return FALSE;
		}
		if(returns.at<int32_t>(0) == -2)
			core.attack_target = 0;
		else
			core.attack_target = return_cards.list[0];
		if(core.attack_target)
			core.pre_field[1] = core.attack_target->fieldid_r;
		else
			core.pre_field[1] = 0;
		return FALSE;
	}
	case 7: {
		if(!core.units.begin()->arg1) {
			core.phase_action = TRUE;
			++core.attack_state_count[infos.turn_player];
			check_card_counter(core.attacker, ACTIVITY_ATTACK, infos.turn_player);
			++core.attacker->attack_announce_count;
		}
		if(core.units.begin()->arg3) {//attack announce failed
			++core.attacker->announce_count;
			core.chain_attack = FALSE;
			if(core.forced_attack) {
				returns.set<int32_t>(0, 2);
				core.units.begin()->step = 0;
			} else
				core.units.begin()->step = -1;
		}
		return FALSE;
	}
	case 8: {
		core.attack_cancelable = TRUE;
		auto message = pduel->new_message(MSG_ATTACK);
		message->write(core.attacker->get_info_location());
		if(core.attack_target) {
			raise_single_event(core.attack_target, 0, EVENT_BE_BATTLE_TARGET, 0, 0, 0, 1 - infos.turn_player, 0);
			raise_event(core.attack_target, EVENT_BE_BATTLE_TARGET, 0, 0, 0, 1 - infos.turn_player, 0);
			message->write(core.attack_target->get_info_location());
		} else {
			message->write(loc_info{});
		}
		core.attack_rollback = FALSE;
		core.opp_mzone.clear();
		for(auto& pcard : player[1 - infos.turn_player].list_mzone) {
			if(pcard)
				core.opp_mzone.insert(pcard->fieldid_r);
		}
		//core.units.begin()->arg1 ---> is rollbacked
		if(!core.units.begin()->arg1) {
			raise_single_event(core.attacker, 0, EVENT_ATTACK_ANNOUNCE, 0, 0, 0, infos.turn_player, 0);
			raise_event(core.attacker, EVENT_ATTACK_ANNOUNCE, 0, 0, 0, infos.turn_player, 0);
		}
		core.attacker->attack_controler = core.attacker->current.controler;
		core.pre_field[0] = core.attacker->fieldid_r;
		process_single_event();
		process_instant_event();
		core.hint_timing[infos.turn_player] = TIMING_ATTACK;
		add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, 0, 0);
		return FALSE;
	}
	case 9: {
		if(is_player_affected_by_effect(infos.turn_player, EFFECT_SKIP_BP)
			|| core.attacker->is_status(STATUS_ATTACK_CANCELED) || core.attack_rollback) {
			core.units.begin()->step = 10;
			return FALSE;
		}
		auto message = pduel->new_message(MSG_HINT);
		message->write<uint8_t>(HINT_EVENT);
		message->write<uint8_t>(0);
		message->write<uint64_t>(24);
		message = pduel->new_message(MSG_HINT);
		message->write<uint8_t>(HINT_EVENT);
		message->write<uint8_t>(1);
		message->write<uint64_t>(24);
		core.hint_timing[0] = TIMING_BATTLE_PHASE;
		core.hint_timing[1] = TIMING_BATTLE_PHASE;
		add_process(PROCESSOR_POINT_EVENT, 30, 0, 0, 0, 0);
		return FALSE;
	}
	case 10: {
		if(returns.at<int32_t>(0))
			core.units.begin()->step = 8;
		else
			adjust_all();
		return FALSE;
	}
	case 11: {
		if(core.attacker->is_affected_by_effect(EFFECT_ATTACK_DISABLED)) {
			core.attacker->reset(EFFECT_ATTACK_DISABLED, RESET_CODE);
			pduel->new_message(MSG_ATTACK_DISABLED);
			core.attacker->set_status(STATUS_ATTACK_CANCELED, TRUE);
		}
		if(is_player_affected_by_effect(infos.turn_player, EFFECT_SKIP_BP)
			|| core.attacker->is_status(STATUS_ATTACK_CANCELED)) {
			core.units.begin()->step = 12;
			return FALSE;
		}
		// go to damage step
		if(!core.attack_rollback) {
			++core.attacker->announce_count;
			core.attacker->announced_cards.addcard(core.attack_target);
			attack_all_target_check();
			core.units.begin()->step = 18;
			return FALSE;
		}
		card_vector cv;
		get_attack_target(core.attacker, &cv, core.chain_attack);
		if(!cv.size() && !core.attacker->direct_attackable) {
			core.units.begin()->step = 12;
			return FALSE;
		}
		// replay
		if(is_flag(DUEL_STORE_ATTACK_REPLAYS)) {
			returns.set<int32_t>(0, FALSE);
		} else if(!core.attacker->is_affected_by_effect(EFFECT_MUST_ATTACK))
			add_process(PROCESSOR_SELECT_YESNO, 0, 0, 0, infos.turn_player, 30);
		else {
			returns.set<int32_t>(0, TRUE);
			core.attack_cancelable = FALSE;
		}
		return FALSE;
	}
	case 12: {
		// answer of "replay or not"
		if(returns.at<int32_t>(0)) {
			core.units.begin()->arg1 = TRUE;
			core.units.begin()->arg3 = FALSE;
			core.units.begin()->step = 3;
		}
		return FALSE;
	}
	case 13: {
		if(core.attacker->fieldid_r == core.pre_field[0] && (!is_flag(DUEL_STORE_ATTACK_REPLAYS) || core.attacker->is_status(STATUS_ATTACK_CANCELED))) {
			++core.attacker->announce_count;
			core.attacker->announced_cards.addcard(core.attack_target);
			attack_all_target_check();
		}
		core.chain_attack = FALSE;
		if(core.forced_attack) {
			returns.set<int32_t>(0, 2);
			core.units.begin()->step = 0;
		} else
			core.units.begin()->step = -1;
		reset_phase(PHASE_DAMAGE);
		adjust_all();
		return FALSE;
	}
	case 14: {
		for(auto clit = core.tpchain.begin(); clit != core.tpchain.end(); ++clit) {
			core.sub_solving_event.push_back(nil_event);
			add_process(PROCESSOR_EXECUTE_OPERATION, 0, clit->triggering_effect, 0, infos.turn_player, 0);
			adjust_all();
		}
		core.tpchain.clear();
		core.units.begin()->step = 2;
		return FALSE;
	}
	case 15: {
		adjust_instant();
		add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, 0, 0);
		core.units.begin()->step = -1;
		return FALSE;
	}
	case 19: {
		infos.phase = PHASE_DAMAGE;
		core.chain_attack = FALSE;
		core.units.begin()->arg1 = FALSE;
		core.damage_calculated = FALSE;
		core.selfdes_disabled = TRUE;
		core.flip_delayed = TRUE;
		core.attacker->attack_controler = core.attacker->current.controler;
		core.pre_field[0] = core.attacker->fieldid_r;
		if(core.attack_target) {
			core.attack_target->attack_controler = core.attack_target->current.controler;
			core.pre_field[1] = core.attack_target->fieldid_r;
		} else
			core.pre_field[1] = 0;
		++core.attacker->attacked_count;
		core.attacker->attacked_cards.addcard(core.attack_target);
		++core.battled_count[infos.turn_player];
		adjust_all();
		return FALSE;
	}
	case 20: {
		// start of PHASE_DAMAGE;
		(void)pduel->new_message(MSG_DAMAGE_STEP_START);
		raise_single_event(core.attacker, 0, EVENT_BATTLE_START, 0, 0, 0, 0, 0);
		if(core.attack_target) {
			raise_single_event(core.attack_target, 0, EVENT_BATTLE_START, 0, 0, 0, 0, 1);
		}
		raise_event((card*)0, EVENT_BATTLE_START, 0, 0, 0, 0, 0);
		process_single_event();
		process_instant_event();
		if(!is_flag(DUEL_6_STEP_BATLLE_STEP) || (core.new_fchain.size() || core.new_ochain.size())) {
			core.units.begin()->arg4 = core.new_fchain.size() || core.new_ochain.size();
			auto message = pduel->new_message(MSG_HINT);
			message->write<uint8_t>(HINT_EVENT);
			message->write<uint8_t>(0);
			message->write<uint64_t>(40);
			message = pduel->new_message(MSG_HINT);
			message->write<uint8_t>(HINT_EVENT);
			message->write<uint8_t>(1);
			message->write<uint64_t>(40);
			add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, 0, is_flag(DUEL_SINGLE_CHAIN_IN_DAMAGE_SUBSTEP));
		}
		return FALSE;
	}
	case 21: {
		if(core.attacker->is_status(STATUS_ATTACK_CANCELED)) {
			core.units.begin()->step = 33;
			return FALSE;
		}
		if(!core.attack_target) {
			return FALSE;
		}
		core.attacker->temp.position = core.attacker->current.position;
		core.attack_target->temp.position = core.attack_target->current.position;
		if(core.attack_target->is_position(POS_FACEDOWN)) {
			change_position(core.attack_target, 0, PLAYER_NONE, core.attack_target->current.position >> 1, 0, TRUE);
			adjust_all();
		}
		return FALSE;
	}
	case 22: {
		raise_single_event(core.attacker, 0, EVENT_BATTLE_CONFIRM, 0, 0, 0, 0, 0);
		if(core.attack_target) {
			if(core.attack_target->temp.position & POS_FACEDOWN)
				core.pre_field[1] = core.attack_target->fieldid_r;
			raise_single_event(core.attack_target, 0, EVENT_BATTLE_CONFIRM, 0, 0, 0, 0, 1);
		}
		raise_event((card*)0, EVENT_BATTLE_CONFIRM, 0, 0, 0, 0, 0);
		process_single_event();
		process_instant_event();
		if(!is_flag(DUEL_6_STEP_BATLLE_STEP) || !core.units.begin()->arg4 || core.new_fchain.size() || core.new_ochain.size()) {
			auto message = pduel->new_message(MSG_HINT);
			message->write<uint8_t>(HINT_EVENT);
			message->write<uint8_t>(0);
			message->write<uint64_t>(41);
			message = pduel->new_message(MSG_HINT);
			message->write<uint8_t>(HINT_EVENT);
			message->write<uint8_t>(1);
			message->write<uint64_t>(41);
			core.hint_timing[infos.turn_player] = TIMING_DAMAGE_STEP;
			add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, 0, is_flag(DUEL_SINGLE_CHAIN_IN_DAMAGE_SUBSTEP));
		}
		return FALSE;
	}
	case 23: {
		if(core.attacker->is_status(STATUS_ATTACK_CANCELED)) {
			core.units.begin()->step = 33;
			return FALSE;
		}
		infos.phase = PHASE_DAMAGE_CAL;
		adjust_all();
		return FALSE;
	}
	case 24: {
		// PHASE_DAMAGE_CAL;
		calculate_battle_damage(0, 0, 0);
		raise_single_event(core.attacker, 0, EVENT_PRE_DAMAGE_CALCULATE, 0, 0, 0, 0, 0);
		if(core.attack_target)
			raise_single_event(core.attack_target, 0, EVENT_PRE_DAMAGE_CALCULATE, 0, 0, 0, 0, 1);
		raise_event((card*)0, EVENT_PRE_DAMAGE_CALCULATE, 0, 0, 0, 0, 0);
		process_single_event();
		process_instant_event();
		auto message = pduel->new_message(MSG_HINT);
		message->write<uint8_t>(HINT_EVENT);
		message->write<uint8_t>(0);
		message->write<uint64_t>(42);
		message = pduel->new_message(MSG_HINT);
		message->write<uint8_t>(HINT_EVENT);
		message->write<uint8_t>(1);
		message->write<uint64_t>(42);
		core.hint_timing[infos.turn_player] = TIMING_DAMAGE_CAL;
		add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, 0, is_flag(DUEL_SINGLE_CHAIN_IN_DAMAGE_SUBSTEP));
		return FALSE;
	}
	case 25: {
		if(core.attacker->is_status(STATUS_ATTACK_CANCELED)) {
			reset_phase(PHASE_DAMAGE_CAL);
			adjust_all();
			infos.phase = PHASE_DAMAGE;
			core.units.begin()->step = 33;
			return FALSE;
		}
		return FALSE;
	}
	case 26: {
		// Duel.CalculateDamage() goes here
		uint32_t aa = core.attacker->get_attack(), ad = core.attacker->get_defense();
		uint32_t da = 0, dd = 0;
		uint8_t pa = core.attacker->current.controler, pd;
		core.attacker->set_status(STATUS_BATTLE_RESULT, FALSE);
		core.attacker->set_status(STATUS_BATTLE_DESTROYED, FALSE);
		if(core.attack_target) {
			da = core.attack_target->get_attack();
			dd = core.attack_target->get_defense();
			core.attack_target->set_status(STATUS_BATTLE_RESULT, FALSE);
			core.attack_target->set_status(STATUS_BATTLE_DESTROYED, FALSE);
			pd = core.attack_target->current.controler;
			if(pa != pd) {
				core.attacker->set_status(STATUS_OPPO_BATTLE, TRUE);
				core.attack_target->set_status(STATUS_OPPO_BATTLE, TRUE);
			}
		}
		effect* damchange = 0;
		card* reason_card = 0;
		uint8_t bd[2];
		calculate_battle_damage(&damchange, &reason_card, bd);
		if(bd[0]) {
			effect* indestructable_effect = core.attacker->is_affected_by_effect(EFFECT_INDESTRUCTABLE_BATTLE, core.attack_target);
			if(indestructable_effect) {
				auto message = pduel->new_message(MSG_HINT);
				message->write<uint8_t>(HINT_CARD);
				message->write<uint8_t>(0);
				message->write<uint64_t>(indestructable_effect->owner->data.code);
				bd[0] = FALSE;
			} else
				core.attacker->set_status(STATUS_BATTLE_RESULT, TRUE);
		}
		if(bd[1]) {
			effect* indestructable_effect = core.attack_target->is_affected_by_effect(EFFECT_INDESTRUCTABLE_BATTLE, core.attacker);
			if(indestructable_effect) {
				auto message = pduel->new_message(MSG_HINT);
				message->write<uint8_t>(HINT_CARD);
				message->write<uint8_t>(0);
				message->write<uint64_t>(indestructable_effect->owner->data.code);
				bd[1] = FALSE;
			} else
				core.attack_target->set_status(STATUS_BATTLE_RESULT, TRUE);
		}
		auto message = pduel->new_message(MSG_BATTLE);
		message->write(core.attacker->get_info_location());
		message->write<uint32_t>(aa);
		message->write<uint32_t>(ad);
		message->write<uint8_t>(bd[0]);
		if(core.attack_target) {
			message->write(core.attack_target->get_info_location());
			message->write<uint32_t>(da);
			message->write<uint32_t>(dd);
			message->write<uint8_t>(bd[1]);
		} else {
			message->write(loc_info{});
			message->write<uint32_t>(0);
			message->write<uint32_t>(0);
			message->write<uint8_t>(0);
		}
		core.units.begin()->peffect = damchange;
		if(reason_card)
			core.temp_var[0] = reason_card->current.controler;
		if(!reason_card)
			core.temp_var[1] = 0;
		else if(reason_card == core.attacker)
			core.temp_var[1] = 1;
		else core.temp_var[1] = 2;
		if(!damchange) {
			if(core.battle_damage[infos.turn_player]) {
				raise_single_event(core.attacker, 0, EVENT_PRE_BATTLE_DAMAGE, 0, 0, reason_card->current.controler, infos.turn_player, core.battle_damage[infos.turn_player]);
				if(core.attack_target)
					raise_single_event(core.attack_target, 0, EVENT_PRE_BATTLE_DAMAGE, 0, 0, reason_card->current.controler, infos.turn_player, core.battle_damage[infos.turn_player]);
				raise_event(reason_card, EVENT_PRE_BATTLE_DAMAGE, 0, 0, reason_card->current.controler, infos.turn_player, core.battle_damage[infos.turn_player]);
			}
			if(core.battle_damage[1 - infos.turn_player]) {
				raise_single_event(core.attacker, 0, EVENT_PRE_BATTLE_DAMAGE, 0, 0, reason_card->current.controler, 1 - infos.turn_player, core.battle_damage[1 - infos.turn_player]);
				if(core.attack_target)
					raise_single_event(core.attack_target, 0, EVENT_PRE_BATTLE_DAMAGE, 0, 0, reason_card->current.controler, 1 - infos.turn_player, core.battle_damage[1 - infos.turn_player]);
				raise_event(reason_card, EVENT_PRE_BATTLE_DAMAGE, 0, 0, reason_card->current.controler, 1 - infos.turn_player, core.battle_damage[1 - infos.turn_player]);
			}
		}
		process_single_event();
		process_instant_event();
		core.damage_calculated = TRUE;
		return FALSE;
	}
	case 27: {
		infos.phase = PHASE_DAMAGE;
		core.hint_timing[infos.turn_player] = 0;
		core.chain_attack = FALSE;
		core.attacker->battled_cards.addcard(core.attack_target);
		if(core.attack_target)
			core.attack_target->battled_cards.addcard(core.attacker);
		uint8_t reason_player = core.temp_var[0];
		card* reason_card = 0;
		if(core.temp_var[1] == 1)
			reason_card = core.attacker;
		else if(core.temp_var[1] == 2)
			reason_card = core.attack_target;
		effect* damchange = core.units.begin()->peffect;
		if(!damchange) {
			if(core.battle_damage[0])
				damage(0, REASON_BATTLE, reason_player, reason_card, 0, core.battle_damage[0]);
			if(core.battle_damage[1])
				damage(0, REASON_BATTLE, reason_player, reason_card, 1, core.battle_damage[1]);
		} else {
			if(core.battle_damage[0])
				damage(damchange, REASON_EFFECT, reason_player, reason_card, 0, core.battle_damage[0]);
			if(core.battle_damage[1])
				damage(damchange, REASON_EFFECT, reason_player, reason_card, 1, core.battle_damage[1]);
		}
		reset_phase(PHASE_DAMAGE_CAL);
		adjust_all();
		return FALSE;
	}
	case 28: {
		card_set des;
		effect* peffect;
		if(core.attacker->is_status(STATUS_BATTLE_RESULT)
		        && core.attacker->current.location == LOCATION_MZONE && core.attacker->fieldid_r == core.pre_field[0]) {
			des.insert(core.attacker);
			core.attacker->temp.reason = core.attacker->current.reason;
			core.attacker->temp.reason_card = core.attacker->current.reason_card;
			core.attacker->temp.reason_effect = core.attacker->current.reason_effect;
			core.attacker->temp.reason_player = core.attacker->current.reason_player;
			core.attacker->current.reason_effect = 0;
			core.attacker->current.reason = REASON_BATTLE;
			core.attacker->current.reason_card = core.attack_target;
			core.attacker->current.reason_player = core.attack_target->current.controler;
			uint32_t dest = LOCATION_GRAVE;
			uint32_t seq = 0;
			if((peffect = core.attack_target->is_affected_by_effect(EFFECT_BATTLE_DESTROY_REDIRECT)) != nullptr && (core.attacker->data.type & TYPE_MONSTER)) {
				dest = peffect->get_value(core.attacker);
				seq = dest >> 16;
				dest &= 0xffff;
			}
			core.attacker->sendto_param.set(core.attacker->owner, POS_FACEUP, dest, seq);
			core.attacker->set_status(STATUS_DESTROY_CONFIRMED, TRUE);
		}
		if(core.attack_target && core.attack_target->is_status(STATUS_BATTLE_RESULT)
		        && core.attack_target->current.location == LOCATION_MZONE && core.attack_target->fieldid_r == core.pre_field[1]) {
			des.insert(core.attack_target);
			core.attack_target->temp.reason = core.attack_target->current.reason;
			core.attack_target->temp.reason_card = core.attack_target->current.reason_card;
			core.attack_target->temp.reason_effect = core.attack_target->current.reason_effect;
			core.attack_target->temp.reason_player = core.attack_target->current.reason_player;
			core.attack_target->current.reason_effect = 0;
			core.attack_target->current.reason = REASON_BATTLE;
			core.attack_target->current.reason_card = core.attacker;
			core.attack_target->current.reason_player = core.attacker->current.controler;
			uint32_t dest = LOCATION_GRAVE;
			uint32_t seq = 0;
			if((peffect = core.attacker->is_affected_by_effect(EFFECT_BATTLE_DESTROY_REDIRECT)) != nullptr && (core.attack_target->data.type & TYPE_MONSTER)) {
				dest = peffect->get_value(core.attack_target);
				seq = dest >> 16;
				dest &= 0xffff;
			}
			core.attack_target->sendto_param.set(core.attack_target->owner, POS_FACEUP, dest, seq);
			core.attack_target->set_status(STATUS_DESTROY_CONFIRMED, TRUE);
		}
		core.attacker->set_status(STATUS_BATTLE_RESULT, FALSE);
		if(core.attack_target)
			core.attack_target->set_status(STATUS_BATTLE_RESULT, FALSE);
		core.battle_destroy_rep.clear();
		core.desrep_chain.clear();
		if(des.size()) {
			group* ng = pduel->new_group();
			ng->container.swap(des);
			ng->is_readonly = TRUE;
			add_process(PROCESSOR_DESTROY, 10, 0, ng, REASON_BATTLE, PLAYER_NONE);
			core.units.begin()->ptarget = ng;
		}
		return FALSE;
	}
	case 29: {
		if(core.battle_destroy_rep.size())
			destroy(core.battle_destroy_rep, 0, REASON_EFFECT | REASON_REPLACE, PLAYER_NONE);
		if(core.desrep_chain.size())
			add_process(PROCESSOR_OPERATION_REPLACE, 15, nullptr, nullptr, 0, 0);
		adjust_all();
		return FALSE;
	}
	case 30: {
		group* des = core.units.begin()->ptarget;
		if(des && des->container.size()) {
			for(auto& pcard : des->container) {
				pcard->set_status(STATUS_BATTLE_DESTROYED, TRUE);
				pcard->set_status(STATUS_DESTROY_CONFIRMED, FALSE);
				pcard->filter_disable_related_cards();
			}
		}
		core.selfdes_disabled = FALSE;
		adjust_all();
		if(is_flag(DUEL_6_STEP_BATLLE_STEP)) {
			//EVENT_BATTLE_END was here, but this timing does not exist in Master Rule 3+
			if(!core.effect_damage_step) {
				auto message = pduel->new_message(MSG_HINT);
				message->write<uint8_t>(HINT_EVENT);
				message->write<uint8_t>(0);
				message->write<uint64_t>(45);
				message = pduel->new_message(MSG_HINT);
				message->write<uint8_t>(HINT_EVENT);
				message->write<uint8_t>(1);
				message->write<uint64_t>(45);
				core.hint_timing[infos.turn_player] = TIMING_DAMAGE_CAL;
				add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, 0, is_flag(DUEL_SINGLE_CHAIN_IN_DAMAGE_SUBSTEP));
			} else {
				break_effect();
			}
		}
		return FALSE;
	}
	case 31: {
		core.flip_delayed = FALSE;
		core.new_fchain.splice(core.new_fchain.begin(), core.new_fchain_b);
		core.new_ochain.splice(core.new_ochain.begin(), core.new_ochain_b);
		raise_single_event(core.attacker, 0, EVENT_BATTLED, 0, 0, PLAYER_NONE, 0, 0);
		if(core.attack_target)
			raise_single_event(core.attack_target, 0, EVENT_BATTLED, 0, 0, PLAYER_NONE, 0, 1);
		raise_event((card*)0, EVENT_BATTLED, 0, 0, PLAYER_NONE, 0, 0);
		process_single_event();
		process_instant_event();
		if(core.effect_damage_step) {
			core.reserved.ptr1 = core.units.begin()->ptarget;
			return TRUE;
		}
		core.units.begin()->step = 32;
	}
	// fall through
	case 32: {
		auto message = pduel->new_message(MSG_HINT);
		message->write<uint8_t>(HINT_EVENT);
		message->write<uint8_t>(0);
		message->write<uint64_t>(43);
		message = pduel->new_message(MSG_HINT);
		message->write<uint8_t>(HINT_EVENT);
		message->write<uint8_t>(1);
		message->write<uint64_t>(43);
		core.hint_timing[0] |= TIMING_BATTLED;
		core.hint_timing[1] |= TIMING_BATTLED;
		add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, 0, is_flag(DUEL_SINGLE_CHAIN_IN_DAMAGE_SUBSTEP));
		return FALSE;
	}
	case 33: {
		group* des = core.units.begin()->ptarget;
		if(des) {
			for(auto cit = des->container.begin(); cit != des->container.end();) {
				auto rm = cit++;
				if((*rm)->current.location != LOCATION_MZONE || ((*rm)->fieldid_r != core.pre_field[0] && (*rm)->fieldid_r != core.pre_field[1]))
					des->container.erase(rm);
			}
			add_process(PROCESSOR_DESTROY, 3, 0, des, REASON_BATTLE, PLAYER_NONE);
		}
		adjust_all();
		return FALSE;
	}
	case 34: {
		core.units.begin()->ptarget = 0;
		// for unexpected end of damage step
		core.damage_calculated = TRUE;
		core.selfdes_disabled = FALSE;
		core.flip_delayed = FALSE;
		core.new_fchain.splice(core.new_fchain.begin(), core.new_fchain_b);
		core.new_ochain.splice(core.new_ochain.begin(), core.new_ochain_b);
		card_set ing;
		card_set ed;
		if(core.attacker->is_status(STATUS_BATTLE_DESTROYED) && (core.attacker->current.reason & REASON_BATTLE)) {
			raise_single_event(core.attack_target, 0, EVENT_BATTLE_DESTROYING, 0, core.attacker->current.reason, core.attack_target->current.controler, 0, 1);
			raise_single_event(core.attacker, 0, EVENT_BATTLE_DESTROYED, 0, core.attacker->current.reason, core.attack_target->current.controler, 0, 0);
			raise_single_event(core.attacker, 0, EVENT_DESTROYED, 0, core.attacker->current.reason, core.attack_target->current.controler, 0, 0);
			ing.insert(core.attack_target);
			ed.insert(core.attacker);
		}
		if(core.attack_target && core.attack_target->is_status(STATUS_BATTLE_DESTROYED) && (core.attack_target->current.reason & REASON_BATTLE)) {
			raise_single_event(core.attacker, 0, EVENT_BATTLE_DESTROYING, 0, core.attack_target->current.reason, core.attacker->current.controler, 0, 0);
			raise_single_event(core.attack_target, 0, EVENT_BATTLE_DESTROYED, 0, core.attack_target->current.reason, core.attacker->current.controler, 0, 1);
			raise_single_event(core.attack_target, 0, EVENT_DESTROYED, 0, core.attack_target->current.reason, core.attacker->current.controler, 0, 1);
			ing.insert(core.attacker);
			ed.insert(core.attack_target);
		}
		if(ing.size())
			raise_event(&ing, EVENT_BATTLE_DESTROYING, 0, 0, 0, 0, 0);
		if(ed.size()) {
			raise_event(&ed, EVENT_BATTLE_DESTROYED, 0, 0, 0, 0, 0);
			raise_event(&ed, EVENT_DESTROYED, 0, 0, 0, 0, 0);
		}
		raise_single_event(core.attacker, 0, EVENT_DAMAGE_STEP_END, 0, 0, 0, 0, 0);
		if(core.attack_target)
			raise_single_event(core.attack_target, 0, EVENT_DAMAGE_STEP_END, 0, 0, 0, 0, 1);
		raise_event((card*)0, EVENT_DAMAGE_STEP_END, 0, 0, 0, 0, 0);
		core.attacker->set_status(STATUS_BATTLE_DESTROYED, FALSE);
		if(core.attack_target)
			core.attack_target->set_status(STATUS_BATTLE_DESTROYED, FALSE);
		process_single_event();
		process_instant_event();
		auto message = pduel->new_message(MSG_HINT);
		message->write<uint8_t>(HINT_EVENT);
		message->write<uint8_t>(0);
		message->write<uint64_t>(44);
		message = pduel->new_message(MSG_HINT);
		message->write<uint8_t>(HINT_EVENT);
		message->write<uint8_t>(1);
		message->write<uint64_t>(44);
		add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, 0, TRUE);
		core.units.begin()->step = 38;
		return FALSE;
	}
	case 38: {
		return FALSE;
	}
	case 39: {
		//end of damage step
		core.attacker->set_status(STATUS_OPPO_BATTLE, FALSE);
		if(core.attack_target)
			core.attack_target->set_status(STATUS_OPPO_BATTLE, FALSE);
		if(core.forced_attack) {
			returns.set<int32_t>(0, 2);
			core.units.begin()->step = 0;
		} else
			core.units.begin()->step = -1;
		infos.phase = PHASE_BATTLE_STEP;
		pduel->new_message(MSG_DAMAGE_STEP_END);
		reset_phase(PHASE_DAMAGE);
		adjust_all();
		if(core.effect_damage_step)
			return TRUE;
		return FALSE;
	}
	case 40: {
		for(auto& ch_lim : core.chain_limit)
			luaL_unref(pduel->lua->lua_state, LUA_REGISTRYINDEX, ch_lim.function);
		core.chain_limit.clear();
		if(core.current_chain.size()) {
			for(auto& ch : core.current_chain)
				ch.triggering_effect->get_handler()->set_status(STATUS_CHAINING, FALSE);
			add_process(PROCESSOR_SOLVE_CHAIN, 0, 0, 0, FALSE, 0);
			if(!core.forced_attack)
				core.units.begin()->step = -1;
			return FALSE;
		}
		reset_phase(PHASE_BATTLE_STEP);
		adjust_all();
		return FALSE;
	}
	case 41: {
		// normal end of battle step
		bool optional_twice_battle = true;
		effect_set eset;
		filter_player_effect(infos.turn_player, EFFECT_BP_TWICE, &eset, FALSE);
		for(const auto& peff : eset) {
			if(!peff->value || peff->get_value() != 1) {
				optional_twice_battle = false;
				break;
			}
		}
		if(eset.size()) {
			core.units.begin()->arg2 = 1;
			core.units.begin()->arg3 = optional_twice_battle ? 1 : 0;
		} else
			core.units.begin()->arg2 = 0;
		infos.phase = PHASE_BATTLE;
		add_process(PROCESSOR_PHASE_EVENT, 0, 0, 0, PHASE_BATTLE, 0);
		adjust_all();
		return FALSE;
	}
	case 42: {
		core.attacker = 0;
		core.attack_target = 0;
		if(core.units.begin()->arg2 && core.units.begin()->arg3) {
			add_process(PROCESSOR_SELECT_YESNO, 0, 0, 0, infos.turn_player, 32);
			return FALSE;
		}
		returns.set<int32_t>(0, core.units.begin()->arg1);
		returns.set<int32_t>(1, core.units.begin()->arg2);
		return TRUE;
	}
	case 43: {
		auto bp_twice = returns.at<int32_t>(0);
		returns.set<int32_t>(0, core.units.begin()->arg1);
		returns.set<int32_t>(1, bp_twice);
		return TRUE;
	}
	}
	return TRUE;
}
int32_t field::process_forced_battle(uint16_t step) {
	switch(step) {
	case 0: {
		if (is_player_affected_by_effect(infos.turn_player, EFFECT_CANNOT_BP))
			return TRUE;
		++core.battle_phase_count[infos.turn_player];
		if (is_player_affected_by_effect(infos.turn_player, EFFECT_SKIP_BP) || core.force_turn_end) {
			auto message = pduel->new_message(MSG_NEW_PHASE);
			message->write<uint16_t>(PHASE_BATTLE_START);
			reset_phase(PHASE_BATTLE_START);
			reset_phase(PHASE_BATTLE_STEP);
			reset_phase(PHASE_BATTLE);
			adjust_all();
			message = pduel->new_message(MSG_NEW_PHASE);
			message->write<uint16_t>(infos.phase);
			return TRUE;
		}
		core.units.begin()->arg1 = infos.phase;
		auto tmp_attacker = core.forced_attacker;
		auto tmp_attack_target = core.forced_attack_target;
		if(!tmp_attacker->is_capable_attack_announce(infos.turn_player))
			return TRUE;
		card_vector cv;
		get_attack_target(tmp_attacker, &cv);
		if((cv.size() == 0 && tmp_attacker->direct_attackable == 0) || (tmp_attack_target && std::find(cv.begin(), cv.end(), tmp_attack_target)==cv.end()))
			return TRUE;
		core.attacker = tmp_attacker;
		core.attack_target = tmp_attack_target;
		for(uint8_t p = 0; p < 2; ++p) {
			for(auto& pcard : player[p].list_mzone) {
				if(!pcard)
					continue;
				pcard->attack_announce_count = 0;
				pcard->announce_count = 0;
				pcard->attacked_count = 0;
				pcard->announced_cards.clear();
				pcard->attacked_cards.clear();
				pcard->battled_cards.clear();
			}
		}
		core.forced_attack = true;
		core.attack_cancelable = TRUE;
		core.attack_cost_paid = FALSE;
		core.chain_attacker_id = 0;
		core.chain_attack_target = 0;
		returns.set<int32_t>(0, 1);
		reset_phase(infos.phase);
		reset_phase(PHASE_BATTLE_START);
		infos.phase = PHASE_BATTLE_STEP;
		core.new_fchain.clear();
		core.new_ochain.clear();
		core.quick_f_chain.clear();
		core.delayed_quick_tmp.clear();
		auto message = pduel->new_message(MSG_NEW_PHASE);
		message->write<uint16_t>(PHASE_BATTLE_START);
		add_process(PROCESSOR_BATTLE_COMMAND, 1, 0, 0, 0, 0);
		return FALSE;
	}
	case 1: {
		reset_phase(infos.phase);
		core.forced_attack = false;
		infos.phase = core.units.begin()->arg1;
		core.new_fchain.clear();
		core.new_ochain.clear();
		core.quick_f_chain.clear();
		core.delayed_quick_tmp.clear();
		for(uint8_t p = 0; p < 2; ++p) {
			for(auto& pcard : player[p].list_mzone) {
				if(!pcard)
					continue;
				pcard->attack_announce_count = 0;
				pcard->announce_count = 0;
				pcard->attacked_count = 0;
				pcard->announced_cards.clear();
				pcard->attacked_cards.clear();
				pcard->battled_cards.clear();
			}
		}
		core.attacker = 0;
		core.attack_target = 0;
		auto message = pduel->new_message(MSG_NEW_PHASE);
		message->write<uint16_t>(infos.phase);
		return TRUE;
	}
	}
	return TRUE;
}
int32_t field::process_damage_step(uint16_t step, uint32_t new_attack) {
	switch(step) {
	case 0: {
		if(core.effect_damage_step && !new_attack)
			return TRUE;
		core.effect_damage_step = 1;
		card* tmp = core.attacker;
		core.attacker = (card*)core.units.begin()->peffect;
		core.units.begin()->peffect = (effect*)tmp;
		tmp = core.attack_target;
		core.attack_target = (card*)core.units.begin()->ptarget;
		core.units.begin()->ptarget = (group*)tmp;
		core.units.begin()->arg1 = infos.phase;
		if(core.attacker->current.location != LOCATION_MZONE || (core.attack_target && core.attack_target->current.location != LOCATION_MZONE)) {
			core.units.begin()->step = 2;
			return FALSE;
		}
		if(new_attack) {
			++core.attack_state_count[infos.turn_player];
			++core.battled_count[infos.turn_player];
			check_card_counter(core.attacker, ACTIVITY_ATTACK, infos.turn_player);
		}
		core.attacker->announced_cards.addcard(core.attack_target);
		attack_all_target_check();
		auto message = pduel->new_message(MSG_ATTACK);
		message->write(core.attacker->get_info_location());
		if(core.attack_target) {
			message->write(core.attack_target->get_info_location());
		} else {
			message->write(loc_info{});
		}
		infos.phase = PHASE_DAMAGE;
		(void)pduel->new_message(MSG_DAMAGE_STEP_START);
		core.pre_field[0] = core.attacker->fieldid_r;
		++core.attacker->attacked_count;
		if(core.attack_target) {
			core.pre_field[1] = core.attack_target->fieldid_r;
			if(core.attack_target->is_position(POS_FACEDOWN)) {
				change_position(core.attack_target, 0, PLAYER_NONE, core.attack_target->current.position >> 1, 0, TRUE);
				adjust_all();
			}
		} else
			core.pre_field[1] = 0;
		return FALSE;
	}
	case 1: {
		infos.phase = PHASE_DAMAGE_CAL;
		add_process(PROCESSOR_BATTLE_COMMAND, 26, 0, 0, 0, 0);
		core.units.begin()->step = 2;
		core.reserved = core.units.front();
		return TRUE;
	}
	case 2: {
		core.effect_damage_step = 2;
		add_process(PROCESSOR_BATTLE_COMMAND, 32, 0, (group*)core.units.begin()->ptr1, 0, 0);
		return FALSE;
	}
	case 3: {
		core.attacker = (card*)core.units.begin()->peffect;
		core.attack_target = (card*)core.units.begin()->ptarget;
		if(core.attacker)
			core.attacker->set_status(STATUS_ATTACK_CANCELED, TRUE);
		if(core.attack_target)
			core.attack_target->set_status(STATUS_ATTACK_CANCELED, TRUE);
		core.effect_damage_step = 0;
		infos.phase = core.units.begin()->arg1;
		return TRUE;
	}
	}
	return TRUE;
}
void field::calculate_battle_damage(effect** pdamchange, card** preason_card, uint8_t* battle_destroyed) {
	uint32_t aa = core.attacker->get_attack(), ad = core.attacker->get_defense();
	uint32_t da = 0, dd = 0, a = aa, d;
	uint8_t pa = core.attacker->current.controler, pd;
	uint8_t damp = 0;
	effect* damchange = 0;
	card* reason_card = 0;
	uint8_t bd[2] = {FALSE, FALSE};
	bool pierce = false;
	core.battle_damage[0] = core.battle_damage[1] = 0;
	if(core.attacker->is_position(POS_FACEUP_DEFENSE)) {
		effect* defattack = core.attacker->is_affected_by_effect(EFFECT_DEFENSE_ATTACK);
		if(defattack && defattack->get_value(core.attacker))
			a = ad;
	}
	effect* battstat = core.attacker->is_affected_by_effect(EFFECT_CHANGE_BATTLE_STAT);
	if(battstat)
		a = battstat->get_value(core.attacker);
	if(core.attack_target) {
		da = core.attack_target->get_attack();
		dd = core.attack_target->get_defense();
		pd = core.attack_target->current.controler;
		battstat = core.attack_target->is_affected_by_effect(EFFECT_CHANGE_BATTLE_STAT);
		if(battstat)
			d = battstat->get_value(core.attack_target);
		else if (core.attack_target->is_position(POS_ATTACK))
			d = da;
		else
			d = dd;
		if(core.attack_target->is_position(POS_ATTACK)) {
			if(a > d) {
				damp = pd;
				core.battle_damage[damp] = a - d;
				reason_card = core.attacker;
				bd[1] = TRUE;
			} else if(a < d) {
				damp = pa;
				core.battle_damage[damp] = d - a;
				reason_card = core.attack_target;
				bd[0] = TRUE;
			} else {
				if(a != 0 || is_flag(DUEL_0_ATK_DESTROYED)) {
					bd[0] = TRUE;
					bd[1] = TRUE;
				}
			}
		} else {
			if(a > d) {
				effect_set eset;
				core.attacker->filter_effect(EFFECT_PIERCE, &eset);
				if(eset.size()) {
					pierce = true;
					uint8_t dp[2] = {};
					for(const auto& peff : eset)
						dp[1 - peff->get_handler_player()] = 1;
					if(dp[0])
						core.battle_damage[0] = a - d;
					if(dp[1])
						core.battle_damage[1] = a - d;
					bool double_damage = false;
					//bool half_damage = false;
					for(const auto& peff : eset) {
						if(peff->get_value() == static_cast<int32_t>(DOUBLE_DAMAGE))
							double_damage = true;
						//if(peff->get_value() == HALF_DAMAGE)
						//	half_damage = true;
					}
					//if(double_damage && half_damage) {
					//	double_damage = false;
					//	half_damage = false;
					//}
					if(double_damage) {
						if(dp[0])
							core.battle_damage[0] *= 2;
						if(dp[1])
							core.battle_damage[1] *= 2;
					}
					//if(half_damage) {
					//	if(dp[0])
					//		core.battle_damage[0] /= 2;
					//	if(dp[1])
					//		core.battle_damage[1] /= 2;
					//}
					bool both = dp[0] && dp[1];
					if(!both) {
						damp = dp[0] ? 0 : 1;
						if(core.attacker->is_affected_by_effect(EFFECT_BOTH_BATTLE_DAMAGE)
							|| core.attack_target->is_affected_by_effect(EFFECT_BOTH_BATTLE_DAMAGE)) {
							core.battle_damage[1 - damp] = core.battle_damage[damp];
							both = true;
						}
					}
					effect* reflect[2] = {};
					if((reflect[pd] = core.attack_target->is_affected_by_effect(EFFECT_REFLECT_BATTLE_DAMAGE, core.attacker)) == nullptr)
						reflect[pd] = is_player_affected_by_effect(pd, EFFECT_REFLECT_BATTLE_DAMAGE);
					if((reflect[1 - pd] = core.attacker->is_affected_by_effect(EFFECT_REFLECT_BATTLE_DAMAGE, core.attack_target)) == nullptr)
						reflect[1 - pd] = is_player_affected_by_effect(1 - pd, EFFECT_REFLECT_BATTLE_DAMAGE);
					bool also[2] = { false, false };
					if(!both
						&& (core.attack_target->is_affected_by_effect(EFFECT_ALSO_BATTLE_DAMAGE)
							|| is_player_affected_by_effect(pd, EFFECT_ALSO_BATTLE_DAMAGE)))
						also[pd] = true;
					if(!both
						&& (core.attacker->is_affected_by_effect(EFFECT_ALSO_BATTLE_DAMAGE)
							|| is_player_affected_by_effect(1 - pd, EFFECT_ALSO_BATTLE_DAMAGE)))
						also[1 - pd] = true;
					if(both) {
						//turn player's effect applies first
						if(reflect[pa] && reflect[pa]->get_handler_player() == pa) {
							core.battle_damage[1 - pa] += core.battle_damage[pa];
							core.battle_damage[pa] = 0;
						} else if(reflect[1 - pa] && reflect[1 - pa]->get_handler_player() == pa) {
							core.battle_damage[pa] += core.battle_damage[1 - pa];
							core.battle_damage[1 - pa] = 0;
						} else if(reflect[pa] && reflect[pa]->get_handler_player() == 1 - pa) {
							core.battle_damage[1 - pa] += core.battle_damage[pa];
							core.battle_damage[pa] = 0;
						} else if(reflect[1 - pa] && reflect[1 - pa]->get_handler_player() == 1 - pa) {
							core.battle_damage[pa] += core.battle_damage[1 - pa];
							core.battle_damage[1 - pa] = 0;
						}
					} else {
						if(reflect[damp]) {
							if(!also[1 - damp]) {
								core.battle_damage[1 - damp] += core.battle_damage[damp];
								core.battle_damage[damp] = 0;
							} else {
								core.battle_damage[1 - damp] += core.battle_damage[damp];
								core.battle_damage[damp] = core.battle_damage[1 - damp];
							}
						} else if(also[damp]) {
							if(!reflect[1 - damp]) {
								core.battle_damage[1 - damp] += core.battle_damage[damp];
							} else {
								core.battle_damage[1 - damp] += core.battle_damage[damp];
								core.battle_damage[damp] += core.battle_damage[1 - damp];
								core.battle_damage[1 - damp] = 0;
							}
						}
					}
					eset.clear();
					core.attacker->filter_effect(EFFECT_CHANGE_BATTLE_DAMAGE, &eset, FALSE);
					core.attack_target->filter_effect(EFFECT_CHANGE_BATTLE_DAMAGE, &eset, FALSE);
					filter_player_effect(pa, EFFECT_CHANGE_BATTLE_DAMAGE, &eset, FALSE);
					filter_player_effect(1 - pa, EFFECT_CHANGE_BATTLE_DAMAGE, &eset, FALSE);
					std::sort(eset.begin(), eset.end(), effect_sort_id);
					for(uint8_t p = 0; p < 2; ++p) {
						bool double_dam = false;
						bool half_dam = false;
						int32_t dam_value = -1;
						for(const auto& peff : eset) {
							int32_t val = -1;
							if(!peff->is_flag(EFFECT_FLAG_PLAYER_TARGET)) {
								pduel->lua->add_param<PARAM_TYPE_INT>(p);
								pduel->lua->add_param<PARAM_TYPE_CARD>(core.attacker);
								val = peff->get_value(2);
							} else if(peff->is_target_player(p)) {
								pduel->lua->add_param<PARAM_TYPE_CARD>(core.attacker);
								val = peff->get_value(1);
							}
							if(val == 0) {
								dam_value = 0;
								break;
							} else if(val > 0)
								dam_value = val;
							else if(val == static_cast<int32_t>(DOUBLE_DAMAGE))
								double_dam = true;
							else if(val == static_cast<int32_t>(HALF_DAMAGE))
								half_dam = true;
						}
						if(double_dam && half_dam) {
							double_dam = false;
							half_dam = false;
						}
						if(double_dam)
							core.battle_damage[p] *= 2;
						if(half_dam)
							core.battle_damage[p] /= 2;
						if(dam_value >= 0 && core.battle_damage[p] > 0)
							core.battle_damage[p] = dam_value;
					}
					if(core.attacker->is_affected_by_effect(EFFECT_NO_BATTLE_DAMAGE)
						|| core.attack_target->is_affected_by_effect(EFFECT_AVOID_BATTLE_DAMAGE, core.attacker)
						|| is_player_affected_by_effect(pd, EFFECT_AVOID_BATTLE_DAMAGE))
						core.battle_damage[pd] = 0;
					if(core.attack_target->is_affected_by_effect(EFFECT_NO_BATTLE_DAMAGE)
						|| core.attacker->is_affected_by_effect(EFFECT_AVOID_BATTLE_DAMAGE, core.attack_target)
						|| is_player_affected_by_effect(1 - pd, EFFECT_AVOID_BATTLE_DAMAGE))
						core.battle_damage[1 - pd] = 0;
					reason_card = core.attacker;
				}
				bd[1] = TRUE;
			} else if(a < d) {
				damp = pa;
				core.battle_damage[damp] = d - a;
				reason_card = core.attack_target;
			}
		}
	} else {
		if(a != 0) {
			damp = 1 - pa;
			core.battle_damage[damp] = a;
			reason_card = core.attacker;
		}
	}
	if(reason_card && !pierce
		&& (damchange = reason_card->is_affected_by_effect(EFFECT_BATTLE_DAMAGE_TO_EFFECT)) == nullptr) {
		card* dam_card = (reason_card == core.attacker) ? core.attack_target : core.attacker;
		bool both = false;
		if(reason_card->is_affected_by_effect(EFFECT_BOTH_BATTLE_DAMAGE)
			|| (dam_card && dam_card->is_affected_by_effect(EFFECT_BOTH_BATTLE_DAMAGE))) {
			core.battle_damage[1 - damp] = core.battle_damage[damp];
			both = true;
		}
		effect* reflect[2] = {};
		if(!dam_card || (reflect[damp] = dam_card->is_affected_by_effect(EFFECT_REFLECT_BATTLE_DAMAGE, reason_card)) == nullptr)
			reflect[damp] = is_player_affected_by_effect(damp, EFFECT_REFLECT_BATTLE_DAMAGE);
		if((reflect[1 - damp] = reason_card->is_affected_by_effect(EFFECT_REFLECT_BATTLE_DAMAGE, dam_card)) == nullptr)
			reflect[1 - damp] = is_player_affected_by_effect(1 - damp, EFFECT_REFLECT_BATTLE_DAMAGE);
		bool also[2] = { false, false };
		if(!both
			&& ((dam_card && dam_card->is_affected_by_effect(EFFECT_ALSO_BATTLE_DAMAGE))
				|| is_player_affected_by_effect(damp, EFFECT_ALSO_BATTLE_DAMAGE)))
			also[damp] = true;
		if(!both
			&& (reason_card->is_affected_by_effect(EFFECT_ALSO_BATTLE_DAMAGE)
				|| is_player_affected_by_effect(1 - damp, EFFECT_ALSO_BATTLE_DAMAGE)))
			also[1 - damp] = true;
		if(both) {
			//turn player's effect applies first
			if(reflect[pa] && reflect[pa]->get_handler_player() == pa) {
				core.battle_damage[1 - pa] += core.battle_damage[pa];
				core.battle_damage[pa] = 0;
			} else if(reflect[1 - pa] && reflect[1 - pa]->get_handler_player() == pa) {
				core.battle_damage[pa] += core.battle_damage[1 - pa];
				core.battle_damage[1 - pa] = 0;
			} else if(reflect[pa] && reflect[pa]->get_handler_player() == 1 - pa) {
				core.battle_damage[1 - pa] += core.battle_damage[pa];
				core.battle_damage[pa] = 0;
			} else if(reflect[1 - pa] && reflect[1 - pa]->get_handler_player() == 1 - pa) {
				core.battle_damage[pa] += core.battle_damage[1 - pa];
				core.battle_damage[1 - pa] = 0;
			}
		} else {
			if(reflect[damp]) {
				if(!also[1 - damp]) {
					core.battle_damage[1 - damp] += core.battle_damage[damp];
					core.battle_damage[damp] = 0;
				} else {
					core.battle_damage[1 - damp] += core.battle_damage[damp];
					core.battle_damage[damp] = core.battle_damage[1 - damp];
				}
			} else if(also[damp]) {
				if(!reflect[1 - damp]) {
					core.battle_damage[1 - damp] += core.battle_damage[damp];
				} else {
					core.battle_damage[1 - damp] += core.battle_damage[damp];
					core.battle_damage[damp] += core.battle_damage[1 - damp];
					core.battle_damage[1 - damp] = 0;
				}
			}
		}
		effect_set eset;
		reason_card->filter_effect(EFFECT_CHANGE_BATTLE_DAMAGE, &eset, FALSE);
		if(dam_card)
			dam_card->filter_effect(EFFECT_CHANGE_BATTLE_DAMAGE, &eset, FALSE);
		filter_player_effect(damp, EFFECT_CHANGE_BATTLE_DAMAGE, &eset, FALSE);
		filter_player_effect(1 - damp, EFFECT_CHANGE_BATTLE_DAMAGE, &eset, FALSE);
		std::sort(eset.begin(), eset.end(), effect_sort_id);
		for(uint8_t p = 0; p < 2; ++p) {
			bool double_dam = false;
			bool half_dam = false;
			int32_t dam_value = -1;
			for(const auto& peff : eset) {
				int32_t val = -1;
				if(!peff->is_flag(EFFECT_FLAG_PLAYER_TARGET)) {
					pduel->lua->add_param<PARAM_TYPE_INT>(p);
					pduel->lua->add_param<PARAM_TYPE_CARD>(reason_card);
					val = peff->get_value(2);
				} else if(peff->is_target_player(p)) {
					pduel->lua->add_param<PARAM_TYPE_CARD>(reason_card);
					val = peff->get_value(1);
				}
				if(val == 0) {
					dam_value = 0;
					break;
				} else if(val > 0)
					dam_value = val;
				else if(val == static_cast<int32_t>(DOUBLE_DAMAGE))
					double_dam = true;
				else if(val == static_cast<int32_t>(HALF_DAMAGE))
					half_dam = true;
			}
			if(double_dam && half_dam) {
				double_dam = false;
				half_dam = false;
			}
			if(double_dam)
				core.battle_damage[p] *= 2;
			if(half_dam)
				core.battle_damage[p] /= 2;
			if(dam_value >= 0 && core.battle_damage[p] > 0)
				core.battle_damage[p] = dam_value;
		}
		if(reason_card->is_affected_by_effect(EFFECT_NO_BATTLE_DAMAGE)
			|| (dam_card && dam_card->is_affected_by_effect(EFFECT_AVOID_BATTLE_DAMAGE, reason_card))
			|| is_player_affected_by_effect(damp, EFFECT_AVOID_BATTLE_DAMAGE))
			core.battle_damage[damp] = 0;
		if((dam_card && dam_card->is_affected_by_effect(EFFECT_NO_BATTLE_DAMAGE))
			|| reason_card->is_affected_by_effect(EFFECT_AVOID_BATTLE_DAMAGE, dam_card)
			|| is_player_affected_by_effect(1 - damp, EFFECT_AVOID_BATTLE_DAMAGE))
			core.battle_damage[1 - damp] = 0;
	}
	if(!core.battle_damage[damp] && !core.battle_damage[1 - damp])
		reason_card = 0;
	if(pdamchange)
		*pdamchange = damchange;
	if(preason_card)
		*preason_card = reason_card;
	if(battle_destroyed) {
		battle_destroyed[0] = bd[0];
		battle_destroyed[1] = bd[1];
	}
}
int32_t field::process_turn(uint16_t step, uint8_t turn_player) {
	switch(step) {
	case 0: {
		//Pre Draw
		for(const auto& ev : core.used_event) {
			if(ev.event_cards)
				pduel->delete_group(ev.event_cards);
		}
		core.used_event.clear();
		for(auto& peffect : core.reseted_effects) {
			pduel->delete_effect(peffect);
		}
		core.reseted_effects.clear();
		core.effect_count_code.clear();
		for(uint8_t p = 0; p < 2; ++p) {
			for(auto& pcard : player[p].list_mzone) {
				if(!pcard)
					continue;
				pcard->set_status(STATUS_SUMMON_TURN, FALSE);
				pcard->set_status(STATUS_FLIP_SUMMON_TURN, FALSE);
				pcard->set_status(STATUS_SPSUMMON_TURN, FALSE);
				pcard->set_status(STATUS_SET_TURN, FALSE);
				pcard->set_status(STATUS_FORM_CHANGED, FALSE);
				pcard->indestructable_effects.clear();
				pcard->attack_announce_count = 0;
				pcard->announce_count = 0;
				pcard->attacked_count = 0;
				pcard->announced_cards.clear();
				pcard->attacked_cards.clear();
				pcard->battled_cards.clear();
				pcard->attack_all_target = TRUE;
			}
			for(auto& pcard : player[p].list_szone) {
				if(!pcard)
					continue;
				pcard->set_status(STATUS_SET_TURN, FALSE);
				pcard->indestructable_effects.clear();
			}
			core.summon_state_count[p] = 0;
			core.normalsummon_state_count[p] = 0;
			core.flipsummon_state_count[p] = 0;
			core.spsummon_state_count[p] = 0;
			core.spsummon_state_count_rst[p] = 0;
			core.attack_state_count[p] = 0;
			core.battle_phase_count[p] = 0;
			core.battled_count[p] = 0;
			core.summon_count[p] = 0;
			core.extra_summon[p] = 0;
			core.spsummon_once_map[p].clear();
			core.spsummon_once_map_rst[p].clear();
		}
		add_process(PROCESSOR_REFRESH_RELAY, 0, 0, 0, 0, 0);
		return FALSE;
	}
	case 1:	{
		core.force_turn_end = false;
		core.spsummon_rst = false;
		for(auto& peffect : effects.rechargeable)
			if(!peffect->is_flag(EFFECT_FLAG_NO_TURN_RESET))
				peffect->recharge();
		auto clear_counter = [](processor::action_counter_t& counter) {
			for(auto& iter : counter)
				iter.second.player_amount[0] = iter.second.player_amount[1] = 0;
		};
		clear_counter(core.summon_counter);
		clear_counter(core.normalsummon_counter);
		clear_counter(core.spsummon_counter);
		clear_counter(core.flipsummon_counter);
		clear_counter(core.attack_counter);
		clear_counter(core.chain_counter);
		if(core.global_flag & GLOBALFLAG_SPSUMMON_COUNT) {
			for(auto& peffect : effects.spsummon_count_eff) {
				card* pcard = peffect->get_handler();
				if(!peffect->is_flag(EFFECT_FLAG_NO_TURN_RESET)) {
					pcard->spsummon_counter[0] = pcard->spsummon_counter[1] = 0;
					pcard->spsummon_counter_rst[0] = pcard->spsummon_counter_rst[1] = 0;
				}
			}
		}
		++infos.turn_id;
		++infos.turn_id_by_player[turn_player];
		infos.turn_player = turn_player;
		auto message = pduel->new_message(MSG_NEW_TURN);
		message->write<uint8_t>(turn_player);
		if(!is_flag(DUEL_RELAY) && infos.turn_id != 1)
			tag_swap(turn_player);
		if(is_player_affected_by_effect(infos.turn_player, EFFECT_SKIP_TURN)) {
			core.units.begin()->step = 18;
			reset_phase(PHASE_DRAW);
			reset_phase(PHASE_STANDBY);
			reset_phase(PHASE_END);
			adjust_all();
			return FALSE;
		}
		infos.phase = PHASE_DRAW;
		core.phase_action = FALSE;
		core.hand_adjusted = FALSE;
		raise_event((card*)0, EVENT_PHASE_START + PHASE_DRAW, 0, 0, 0, turn_player, 0);
		process_instant_event();
		adjust_all();
		return FALSE;
	}
	case 2: {
		core.new_fchain.clear();
		core.new_ochain.clear();
		core.quick_f_chain.clear();
		core.delayed_quick_tmp.clear();
		if(is_player_affected_by_effect(infos.turn_player, EFFECT_SKIP_DP) || core.force_turn_end) {
			core.units.begin()->step = 3;
			reset_phase(PHASE_DRAW);
			adjust_all();
			return FALSE;
		}
		auto message = pduel->new_message(MSG_NEW_PHASE);
		message->write<uint16_t>(infos.phase);
		raise_event((card*)0, EVENT_PREDRAW, 0, 0, 0, turn_player, 0);
		process_instant_event();
		message = pduel->new_message(MSG_HINT);
		message->write<uint8_t>(HINT_EVENT);
		message->write<uint8_t>(turn_player);
		message->write<uint64_t>(27);
		if(core.new_fchain.size() || core.new_ochain.size())
			add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, 0, 0);
		/*if(core.set_forced_attack)
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
		return FALSE;
	}
	case 3: {
		// Draw, new ruling
		if(is_flag(DUEL_1ST_TURN_DRAW) || (infos.turn_id > 1)) {
			int32_t count = get_draw_count(infos.turn_player);
			if(count > 0 && is_flag(DUEL_DRAW_UNTIL_5)) {
				count = std::max<int32_t>(static_cast<int32_t>(5 - player[turn_player].list_hand.size()), count);
			}
			if(count > 0) {
				draw(0, REASON_RULE, turn_player, turn_player, count);
				add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, 0, 0);
			}
		}
		add_process(PROCESSOR_PHASE_EVENT, 0, 0, 0, PHASE_DRAW, 0);
		/*if(core.set_forced_attack)
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
		return FALSE;
	}
	case 4: {
		// EVENT_PHASE_PRESTART is removed
		return FALSE;
	}
	case 5: {
		//Standby Phase
		infos.phase = PHASE_STANDBY;
		core.phase_action = FALSE;
		core.new_fchain.clear();
		core.new_ochain.clear();
		core.quick_f_chain.clear();
		core.delayed_quick_tmp.clear();
		if(is_flag(DUEL_NO_STANDBY_PHASE) || is_player_affected_by_effect(infos.turn_player, EFFECT_SKIP_SP) || core.force_turn_end) {
			core.units.begin()->step = 6;
			reset_phase(PHASE_STANDBY);
			adjust_all();
			return FALSE;
		}
		auto message = pduel->new_message(MSG_NEW_PHASE);
		message->write<uint16_t>(infos.phase);
		raise_event((card*)0, EVENT_PHASE_START + PHASE_STANDBY, 0, 0, 0, turn_player, 0);
		process_instant_event();
		/*if(core.set_forced_attack)
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
		return FALSE;
	}
	case 6: {
		// EVENT_PHASE_START + PHASE_STANDBY is a special case(c89642993)
		if(core.new_fchain.size() || core.new_ochain.size() || core.instant_event.back().event_code != EVENT_PHASE_START + PHASE_STANDBY)
			add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, 0, 0);
		add_process(PROCESSOR_PHASE_EVENT, 0, 0, 0, PHASE_STANDBY, 0);
		/*if(core.set_forced_attack)
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
		return FALSE;
	}
	case 7: {
		//Main1
		infos.phase = PHASE_MAIN1;
		core.phase_action = FALSE;
		raise_event((card*)0, EVENT_PHASE_START + PHASE_MAIN1, 0, 0, 0, turn_player, 0);
		process_instant_event();
		adjust_all();
		/*if(core.set_forced_attack)
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
		return FALSE;
	}
	case 8: {
		return FALSE;
	}
	case 9: {
		core.new_fchain.clear();
		core.new_ochain.clear();
		core.quick_f_chain.clear();
		core.delayed_quick_tmp.clear();
		auto message = pduel->new_message(MSG_NEW_PHASE);
		message->write<uint16_t>(infos.phase);
		add_process(PROCESSOR_IDLE_COMMAND, 0, 0, 0, 0, 0);
		/*if(core.set_forced_attack)
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
		return FALSE;
	}
	case 10: {
		if(returns.at<int32_t>(0) == 7) { // End Phase
			core.units.begin()->step = 15;
			return FALSE;
		}
		infos.phase = PHASE_BATTLE_START;
		core.new_fchain.clear();
		core.new_ochain.clear();
		core.quick_f_chain.clear();
		core.delayed_quick_tmp.clear();
		core.phase_action = FALSE;
		++core.battle_phase_count[infos.turn_player];
		auto message = pduel->new_message(MSG_NEW_PHASE);
		message->write<uint16_t>(infos.phase);
		// Show the texts to indicate that BP is entered and skipped
		if(is_player_affected_by_effect(infos.turn_player, EFFECT_SKIP_BP) || core.force_turn_end) {
			core.units.begin()->step = 15;
			reset_phase(PHASE_BATTLE_START);
			reset_phase(PHASE_BATTLE_STEP);
			reset_phase(PHASE_BATTLE);
			adjust_all();
			/*if(core.set_forced_attack)
				add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
			return FALSE;
		}
		raise_event((card*)0, EVENT_PHASE_START + PHASE_BATTLE_START, 0, 0, 0, turn_player, 0);
		process_instant_event();
		adjust_all();
		/*if(core.set_forced_attack)
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
		return FALSE;
	}
	case 11: {
		if(core.new_fchain.size() || core.new_ochain.size())
			add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, 0, 0);
		add_process(PROCESSOR_PHASE_EVENT, 0, 0, 0, PHASE_BATTLE_START, 0);
		/*if(core.set_forced_attack)
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
		return FALSE;
	}
	case 12: {
		infos.phase = PHASE_BATTLE_STEP;
		core.new_fchain.clear();
		core.new_ochain.clear();
		core.quick_f_chain.clear();
		core.delayed_quick_tmp.clear();
		core.phase_action = FALSE;
		core.chain_attack = FALSE;
		add_process(PROCESSOR_BATTLE_COMMAND, 0, 0, 0, 0, 0);
		/*if(core.set_forced_attack)
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
		return FALSE;
	}
	case 13: {
		if(core.units.begin()->arg2 == 0 && returns.at<int32_t>(1)) { // 2nd Battle Phase
			core.units.begin()->arg2 = 1;
			core.units.begin()->step = 9;
			for(uint8_t p = 0; p < 2; ++p) {
				for(auto& pcard : player[p].list_mzone) {
					if(!pcard)
						continue;
					pcard->attack_announce_count = 0;
					pcard->announce_count = 0;
					pcard->attacked_count = 0;
					pcard->announced_cards.clear();
					pcard->attacked_cards.clear();
					pcard->battled_cards.clear();
				}
			}
			return FALSE;
		}
		core.units.begin()->arg2 = 0;
		if(is_flag(DUEL_NO_MAIN_PHASE_2)) {
			core.units.begin()->step = 15;
			adjust_all();
			/*if(core.set_forced_attack)
				add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
			return FALSE;
		}
		core.skip_m2 = FALSE;
		if(returns.at<int32_t>(0) == 3 || core.force_turn_end) { // End Phase
			core.skip_m2 = TRUE;
		}
		//Main2
		infos.phase = PHASE_MAIN2;
		core.phase_action = FALSE;
		raise_event((card*)0, EVENT_PHASE_START + PHASE_MAIN2, 0, 0, 0, turn_player, 0);
		process_instant_event();
		adjust_all();
		/*if(core.set_forced_attack)
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
		return FALSE;
	}
	case 14: {
		if(core.new_fchain.size() || core.new_ochain.size())
			add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, 0, 0);
		/*if(core.set_forced_attack)
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
		return FALSE;
	}
	case 15: {
		core.new_fchain.clear();
		core.new_ochain.clear();
		core.quick_f_chain.clear();
		core.delayed_quick_tmp.clear();
		auto message = pduel->new_message(MSG_NEW_PHASE);
		message->write<uint16_t>(infos.phase);
		infos.can_shuffle = TRUE;
		add_process(PROCESSOR_IDLE_COMMAND, 0, 0, 0, 0, 0);
		/*if(core.set_forced_attack)
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
		return FALSE;
	}
	case 16: {
		//End Phase
		infos.phase = PHASE_END;
		core.phase_action = FALSE;
		if(is_player_affected_by_effect(infos.turn_player, EFFECT_SKIP_EP)) {
			core.units.begin()->step = 18;
			reset_phase(PHASE_END);
			adjust_all();
			return FALSE;
		}
		auto message = pduel->new_message(MSG_NEW_PHASE);
		message->write<uint16_t>(infos.phase);
		raise_event((card*)0, EVENT_PHASE_START + PHASE_END, 0, 0, 0, turn_player, 0);
		process_instant_event();
		adjust_all();
		/*if(core.set_forced_attack)
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
		return FALSE;
	}
	case 17: {
		if(core.new_fchain.size() || core.new_ochain.size())
			add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, 0, 0);
		/*if(core.set_forced_attack)
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
		return FALSE;
	}
	case 18: {
		core.new_fchain.clear();
		core.new_ochain.clear();
		core.quick_f_chain.clear();
		core.delayed_quick_tmp.clear();
		add_process(PROCESSOR_PHASE_EVENT, 0, 0, 0, PHASE_END, 0);
		/*if(core.set_forced_attack)
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
		return FALSE;
	}
	case 19: {
		raise_event((card*)0, EVENT_TURN_END, 0, 0, 0, turn_player, 0);
		process_instant_event();
		adjust_all();
		/*if(core.set_forced_attack)
			add_process(PROCESSOR_FORCED_BATTLE, 0, 0, 0, 0, 0);*/
		return FALSE;
	}
	case 20: {
		core.new_fchain.clear();
		core.new_ochain.clear();
		core.quick_f_chain.clear();
		core.delayed_quick_tmp.clear();
		core.units.begin()->step = -1;
		core.units.begin()->arg1 = 1 - core.units.begin()->arg1;
		return FALSE;
	}
	}
	return TRUE;
}
int32_t field::add_chain(uint16_t step) {
	switch (step) {
	case 0: {
		if (!core.new_chains.size())
			return TRUE;
		auto& clit = core.new_chains.front();
		effect* peffect = clit.triggering_effect;
		core.units.begin()->arg4 = 0;
		// if it's an activate effect, go to subroutine checking for effects allowing the card
		// to be activated even when it shouldn't normally (the turn it was set, from the hand, etc)
		if((peffect->type & EFFECT_TYPE_ACTIVATE) != 0)
			core.units.begin()->step = 9;
		return FALSE;
	}
	case 1: {
		auto& clit = core.new_chains.front();
		effect_set eset;
		filter_player_effect(clit.triggering_player, EFFECT_ACTIVATE_COST, &eset);
		for(const auto& peff : eset) {
			pduel->lua->add_param<PARAM_TYPE_EFFECT>(peff);
			pduel->lua->add_param<PARAM_TYPE_EFFECT>(clit.triggering_effect);
			pduel->lua->add_param<PARAM_TYPE_INT>(clit.triggering_player);
			if(!pduel->lua->check_condition(peff->target, 3))
				continue;
			if(peff->operation) {
				core.sub_solving_event.push_back(clit.evt);
				add_process(PROCESSOR_EXECUTE_OPERATION, 0, peff, 0, clit.triggering_player, 0);
			}
		}
		if(core.units.begin()->arg4 == 0)
			return FALSE;
		effect* peffect = clit.triggering_effect;
		card* phandler = peffect->get_handler();
		phandler->set_status(STATUS_ACT_FROM_HAND, phandler->current.location == LOCATION_HAND);
		if(phandler->current.location == LOCATION_SZONE) {
			change_position(phandler, 0, phandler->current.controler, POS_FACEUP, 0);
		} else {
			uint32_t zone = 0xff;
			if(!(phandler->data.type & (TYPE_FIELD | TYPE_PENDULUM)) && peffect->is_flag(EFFECT_FLAG_LIMIT_ZONE)) {
				pduel->lua->add_param<PARAM_TYPE_INT>(clit.triggering_player);
				pduel->lua->add_param<PARAM_TYPE_GROUP>(clit.evt.event_cards);
				pduel->lua->add_param<PARAM_TYPE_INT>(clit.evt.event_player);
				pduel->lua->add_param<PARAM_TYPE_INT>(clit.evt.event_value);
				pduel->lua->add_param<PARAM_TYPE_EFFECT>(clit.evt.reason_effect);
				pduel->lua->add_param<PARAM_TYPE_INT>(clit.evt.reason);
				pduel->lua->add_param<PARAM_TYPE_INT>(clit.evt.reason_player);
				zone = peffect->get_value(7);
				if(!zone)
					return TRUE;
			}
			int32_t loc = LOCATION_SZONE;
			if(peffect->is_flag(EFFECT_FLAG2_FORCE_ACTIVATE_LOCATION)) {
				loc = peffect->get_value();
				if(!loc)
					return TRUE;
			} else if(phandler->current.location == LOCATION_HAND) {
				if(phandler->data.type & TYPE_PENDULUM) {
					loc = LOCATION_PZONE;
				} else if(phandler->data.type & TYPE_FIELD) {
					loc = LOCATION_FZONE;
				}
			}
			phandler->enable_field_effect(false);
			move_to_field(phandler, phandler->current.controler, phandler->current.controler, loc, (loc == LOCATION_MZONE) ? POS_FACEUP_ATTACK : POS_FACEUP, FALSE, 0, zone);
		}
		return FALSE;
	}
	case 2: {
		auto& clit = core.new_chains.front();
		effect* peffect = clit.triggering_effect;
		card* phandler = peffect->get_handler();
		if(peffect->type & EFFECT_TYPE_ACTIVATE) {
			clit.set_triggering_state(phandler);
		}
		auto message = pduel->new_message(MSG_CHAINING);
		message->write<uint32_t>(phandler->data.code);
		message->write(phandler->get_info_location());
		message->write<uint8_t>(clit.triggering_controler);
		message->write<uint8_t>((uint8_t)clit.triggering_location);
		message->write<uint32_t>(clit.triggering_sequence);
		message->write<uint64_t>(peffect->description);
		message->write<uint32_t>(core.current_chain.size() + 1);
		for(auto& ch_lim : core.chain_limit)
			luaL_unref(pduel->lua->lua_state, LUA_REGISTRYINDEX, ch_lim.function);
		core.chain_limit.clear();
		peffect->card_type = phandler->get_type();
		if((peffect->card_type & (TYPE_TRAP | TYPE_MONSTER)) == (TYPE_TRAP | TYPE_MONSTER))
			peffect->card_type -= TYPE_TRAP;
		if(is_flag(DUEL_TRIGGER_WHEN_PRIVATE_KNOWLEDGE)) {
			if(!((peffect->type & EFFECT_TYPE_CONTINUOUS) == 0 && (peffect->type & EFFECT_TYPE_SINGLE) != 0))
				peffect->set_active_type();
		} else
			peffect->set_active_type();
		peffect->active_handler = peffect->handler->overlay_target;
		clit.chain_count = static_cast<uint8_t>(core.current_chain.size()) + 1;
		clit.target_cards = 0;
		clit.target_player = PLAYER_NONE;
		clit.target_param = 0;
		clit.disable_reason = 0;
		clit.disable_player = PLAYER_NONE;
		clit.replace_op = 0;
		if(phandler->current.location == LOCATION_HAND)
			clit.flag |= CHAIN_HAND_EFFECT;
		core.current_chain.push_back(clit);
		core.current_chain.back().applied_chain_counters = check_chain_counter(peffect, clit.triggering_player, clit.chain_count);
		// triggered events which are not caused by RaiseEvent create relation with the handler
		if(!peffect->is_flag(EFFECT_FLAG_FIELD_ONLY) && (!(peffect->type & 0x2a0) || (peffect->code & EVENT_PHASE) == EVENT_PHASE)) {
			phandler->create_relation(clit);
		}
		peffect->effect_owner = clit.triggering_player;
		// DISABLE_CHAIN should be check before cost
		effect* deffect;
		if(!peffect->is_flag(EFFECT_FLAG_FIELD_ONLY) && phandler->is_has_relation(clit) && (deffect = phandler->is_affected_by_effect(EFFECT_DISABLE_EFFECT)) != nullptr) {
			effect* negeff = pduel->new_effect();
			negeff->owner = deffect->owner;
			negeff->type = EFFECT_TYPE_SINGLE;
			negeff->code = EFFECT_DISABLE_CHAIN;
			negeff->value = clit.chain_id;
			negeff->reset_flag = RESET_CHAIN | RESET_EVENT | deffect->get_value();
			phandler->add_effect(negeff);
		}
		core.new_chains.pop_front();
		return FALSE;
	}
	case 3: {
		auto& clit = core.current_chain.back();
		int32_t playerid = clit.triggering_player;
		effect* peffect = clit.triggering_effect;
		if(!is_flag(DUEL_USE_TRAPS_IN_NEW_CHAIN) && get_cteffect(peffect, playerid, TRUE)) {
			const bool damage_step = infos.phase == PHASE_DAMAGE && !peffect->is_flag(EFFECT_FLAG_DAMAGE_STEP);
			const bool damage_cal = infos.phase == PHASE_DAMAGE_CAL && !peffect->is_flag(EFFECT_FLAG_DAMAGE_CAL);
			if(damage_step || damage_cal) {
				returns.set<int32_t>(0, TRUE);
				return FALSE;
			}
			add_process(PROCESSOR_SELECT_EFFECTYN, 0, 0, (group*)peffect->get_handler(), playerid, 94);
		} else
			returns.set<int32_t>(0, FALSE);
		return FALSE;
	}
	case 4: {
		if(!returns.at<int32_t>(0)) {
			core.select_chains.clear();
			core.select_options.clear();
			core.units.begin()->step = 5;
			return FALSE;
		}
		if(core.select_chains.size() > 1) {
			auto& clit = core.current_chain.back();
			add_process(PROCESSOR_SELECT_OPTION, 0, 0, 0, clit.triggering_player, 0);
		} else
			returns.set<int32_t>(0, 0);
		return FALSE;
	}
	case 5: {
		auto& clit = core.current_chain.back();
		chain& ch = core.select_chains[returns.at<int32_t>(0)];
		int32_t playerid = clit.triggering_player;
		effect* peffect = ch.triggering_effect;
		card* phandler = peffect->get_handler();
		auto message = pduel->new_message(MSG_HINT);
		message->write<uint8_t>(HINT_OPSELECTED);
		message->write<uint8_t>(playerid);
		message->write<uint64_t>(returns.at<int32_t>(0) >= (int32_t)core.select_options.size() ? core.select_options[returns.at<int32_t>(0)] : 65);
		clit.triggering_effect = peffect;
		clit.evt = ch.evt;
		phandler->create_relation(clit);
		peffect->dec_count(playerid);
		if(!(peffect->type & EFFECT_TYPE_ACTIVATE)) {
			peffect->type |= EFFECT_TYPE_ACTIVATE;
			clit.flag |= CHAIN_ACTIVATING;
		}
		core.select_chains.clear();
		core.select_options.clear();
		effect* deffect = pduel->new_effect();
		deffect->owner = phandler;
		deffect->code = 0;
		deffect->type = EFFECT_TYPE_SINGLE;
		deffect->flag[0] = EFFECT_FLAG_CANNOT_DISABLE | EFFECT_FLAG_CLIENT_HINT;
		deffect->description = 65;
		deffect->reset_flag = RESET_CHAIN;
		phandler->add_effect(deffect);
		return FALSE;
	}
	case 6: {
		auto& clit = core.current_chain.back();
		effect* peffect = clit.triggering_effect;
		if(peffect->cost) {
			core.sub_solving_event.push_back(clit.evt);
			add_process(PROCESSOR_EXECUTE_COST, 0, peffect, 0, clit.triggering_player, 0);
		}
		return FALSE;
	}
	case 7: {
		auto& clit = core.current_chain.back();
		effect* peffect = clit.triggering_effect;
		if(peffect->target) {
			core.sub_solving_event.push_back(clit.evt);
			add_process(PROCESSOR_EXECUTE_TARGET, 0, peffect, 0, clit.triggering_player, 0);
		}
		return FALSE;
	}
	case 8: {
		break_effect(false);
		auto& clit = core.current_chain.back();
		effect* peffect = clit.triggering_effect;
		card* phandler = peffect->get_handler();
		if(clit.target_cards && clit.target_cards->container.size()) {
			if(peffect->is_flag(EFFECT_FLAG_CARD_TARGET)) {
				for(auto& pcard : clit.target_cards->container)
					raise_single_event(pcard, 0, EVENT_BECOME_TARGET, peffect, 0, clit.triggering_player, 0, clit.chain_count);
				process_single_event();
				if(clit.target_cards->container.size())
					raise_event(&clit.target_cards->container, EVENT_BECOME_TARGET, peffect, 0, clit.triggering_player, clit.triggering_player, clit.chain_count);
			}
		}
		if(peffect->type & EFFECT_TYPE_ACTIVATE) {
			core.leave_confirmed.insert(phandler);
			if(!(phandler->data.type & (TYPE_CONTINUOUS | TYPE_FIELD | TYPE_EQUIP | TYPE_PENDULUM | TYPE_LINK))
			        && !phandler->is_affected_by_effect(EFFECT_REMAIN_FIELD))
				phandler->set_status(STATUS_LEAVE_CONFIRMED, TRUE);
		}
		if((phandler->get_type() & (TYPE_SPELL | TYPE_TRAP))
				&& (phandler->get_type() & (TYPE_CONTINUOUS | TYPE_FIELD | TYPE_EQUIP | TYPE_PENDULUM | TYPE_LINK))
				&& phandler->is_has_relation(clit) && phandler->current.location == LOCATION_SZONE
				&& !peffect->is_flag(EFFECT_FLAG_FIELD_ONLY))
			clit.flag |= CHAIN_CONTINUOUS_CARD;
		core.phase_action = TRUE;
		if(clit.opinfos.count(0x200) && clit.opinfos[0x200].op_count) {
			if(is_flag(DUEL_CANNOT_SUMMON_OATH_OLD)) {
				core.spsummon_rst = true;
				set_spsummon_counter(clit.triggering_player, true, true);
				if(clit.opinfos[0x200].op_player == PLAYER_ALL)
					set_spsummon_counter(1 - clit.triggering_player, true, true);
			}
			if(is_flag(DUEL_SPSUMMON_ONCE_OLD_NEGATE) && (core.global_flag & GLOBALFLAG_SPSUMMON_ONCE)) {
				auto& optarget = clit.opinfos[0x200];
				if(optarget.op_cards) {
					if(optarget.op_player == PLAYER_ALL) {
						auto opit = optarget.op_cards->container.begin();
						uint32_t sumplayer = optarget.op_param;
						if((*opit)->spsummon_code) {
							++core.spsummon_once_map[sumplayer][(*opit)->spsummon_code];
							++core.spsummon_once_map_rst[sumplayer][(*opit)->spsummon_code];
						}
						++opit;
						if((*opit)->spsummon_code) {
							++core.spsummon_once_map[1 - sumplayer][(*opit)->spsummon_code];
							++core.spsummon_once_map_rst[1 - sumplayer][(*opit)->spsummon_code];
						}
					} else {
						uint32_t sumplayer = clit.triggering_player;
						// genarally setting op_player is unnecessary when the effect targets cards
						// in the case of CATEGORY_SPECIAL_SUMMON(with EFFECT_FLAG_CARD_TARGET), op_player=0x10
						// indecates that it is the opponent that special summons the target monsters
						if(peffect->is_flag(EFFECT_FLAG_CARD_TARGET) && optarget.op_player == 0x10)
							sumplayer = 1 - sumplayer;
						for(auto& pcard : optarget.op_cards->container) {
							if(pcard->spsummon_code) {
								++core.spsummon_once_map[sumplayer][pcard->spsummon_code];
								++core.spsummon_once_map_rst[sumplayer][pcard->spsummon_code];
							}
						}
					}
				}
			}
		}
		auto message = pduel->new_message(MSG_CHAINED);
		message->write<uint8_t>(clit.chain_count);
		raise_event(phandler, EVENT_CHAINING, peffect, 0, clit.triggering_player, clit.triggering_player, clit.chain_count);
		process_instant_event();
		core.just_sent_cards.clear();
		++core.real_chain_count;
		if(core.new_chains.size())
			add_process(PROCESSOR_ADD_CHAIN, 0, 0, 0, 0, 0);
		adjust_all();
		return TRUE;
	}
	case 10: {
		core.units.begin()->arg4 = 1;
		auto& clit = core.new_chains.front();
		effect* peffect = clit.triggering_effect;
		card* phandler = peffect->get_handler();
		int32_t ecode = 0;
		if(phandler->current.location == LOCATION_HAND) {
			if(phandler->data.type & TYPE_TRAP)
				ecode = EFFECT_TRAP_ACT_IN_HAND;
			else if((phandler->data.type & TYPE_SPELL) && (phandler->data.type & TYPE_QUICKPLAY || phandler->is_affected_by_effect(EFFECT_BECOME_QUICK))
					&& infos.turn_player != phandler->current.controler)
				ecode = EFFECT_QP_ACT_IN_NTPHAND;
		} else if(phandler->current.location == LOCATION_SZONE) {
			if((phandler->data.type & TYPE_TRAP) && phandler->get_status(STATUS_SET_TURN))
				ecode = EFFECT_TRAP_ACT_IN_SET_TURN;
			if((phandler->data.type & TYPE_SPELL) && (phandler->data.type & TYPE_QUICKPLAY || phandler->is_affected_by_effect(EFFECT_BECOME_QUICK)) && phandler->get_status(STATUS_SET_TURN))
				ecode = EFFECT_QP_ACT_IN_SET_TURN;
		}
		if(ecode) {
			core.select_effects.clear();
			core.select_options.clear();
			effect_set eset;
			phandler->filter_effect(ecode, &eset);
			if(!eset.empty()) {
				for(const auto& peff : eset) {
					if(peff->check_count_limit(phandler->current.controler)) {
						core.select_effects.push_back(peff);
						core.select_options.push_back(peff->description);
					}
				}
				if(core.select_options.size() == 1)
					returns.set<int32_t>(0, 0);
				else
					add_process(PROCESSOR_SELECT_OPTION, 0, 0, 0, phandler->current.controler, 0);
			}
		} else {
			core.units.begin()->step = 0;
		}
		return FALSE;
	}
	case 11: {
		auto& clit = core.new_chains.front();
		effect* peffect = clit.triggering_effect;
		card* phandler = peffect->get_handler();
		if(!core.select_effects.empty()) {
			auto* eff = core.select_effects[returns.at<int32_t>(0)];
			eff->dec_count(phandler->current.controler);
			pduel->lua->add_param<PARAM_TYPE_EFFECT>(peffect);
			eff->get_value(phandler, 1);
		}
		core.units.begin()->step = 0;
		return FALSE;
	}
	}
	return TRUE;
}
int32_t field::sort_chain(uint16_t step, uint8_t tp) {
	switch(step) {
	case 0: {
		core.select_cards.clear();
		if(tp)
			for(auto clit = core.tpchain.begin(); clit != core.tpchain.end(); ++clit)
				core.select_cards.push_back(clit->triggering_effect->get_handler());
		else
			for(auto clit = core.ntpchain.begin(); clit != core.ntpchain.end(); ++clit)
				core.select_cards.push_back(clit->triggering_effect->get_handler());
		add_process(PROCESSOR_SORT_CARD, 0, 0, 0, tp ? infos.turn_player : (1 - infos.turn_player), 1);
		return FALSE;
	}
	case 1: {
		if(returns.at<int8_t>(0) == -1)
			return TRUE;
		if(tp) {
			chain_array ch(core.tpchain.size());
			int i = 0;
			for(const auto& _ch : core.tpchain)
				ch[returns.at<int8_t>(i++)] = _ch;
			core.tpchain.clear();
			for(const auto& _ch : ch)
				core.tpchain.push_back(_ch);
		} else {
			chain_array ch(core.ntpchain.size());
			int i = 0;
			for(const auto& _ch : core.ntpchain)
				ch[returns.at<int8_t>(i++)] = _ch;
			core.ntpchain.clear();
			for(const auto& _ch : ch)
				core.ntpchain.push_back(_ch);
		}
		return TRUE;
	}
	}
	return TRUE;
}
void field::solve_continuous(uint8_t playerid, effect* peffect, const tevent& e) {
	core.sub_solving_continuous.emplace_back();
	chain& newchain = core.sub_solving_continuous.back();
	newchain.chain_id = 0;
	newchain.chain_count = 0;
	newchain.triggering_effect = peffect;
	newchain.triggering_player = playerid;
	newchain.evt = e;
	newchain.target_cards = 0;
	newchain.target_player = PLAYER_NONE;
	newchain.target_param = 0;
	newchain.disable_player = PLAYER_NONE;
	newchain.disable_reason = 0;
	newchain.flag = 0;
	add_process(PROCESSOR_SOLVE_CONTINUOUS, 0, 0, 0, 0, 0);
}
int32_t field::solve_continuous(uint16_t step) {
	switch(step) {
	case 0: {
		core.solving_continuous.splice(core.solving_continuous.begin(), core.sub_solving_continuous);
		auto& clit = core.solving_continuous.front();
		effect* peffect = clit.triggering_effect;
		uint8_t triggering_player = clit.triggering_player;
		if(!peffect->check_count_limit(triggering_player)) {
			core.solving_continuous.pop_front();
			return TRUE;
		}
		core.continuous_chain.push_back(clit);
		if(peffect->is_flag(EFFECT_FLAG_DELAY) || (!(peffect->code & 0x10030000) && (peffect->code & (EVENT_PHASE | EVENT_PHASE_START))))
			core.conti_solving = TRUE;
		core.units.begin()->ptarget = (group*)core.reason_effect;
		core.units.begin()->arg2 = core.reason_player;
		if(!peffect->target)
			return FALSE;
		core.sub_solving_event.push_back(clit.evt);
		add_process(PROCESSOR_EXECUTE_TARGET, 0, peffect, 0, triggering_player, 0);
		return FALSE;
	}
	case 1: {
		return FALSE;
	}
	case 2: {
		auto& clit = core.solving_continuous.front();
		effect* peffect = clit.triggering_effect;
		uint8_t triggering_player = clit.triggering_player;
		if(!peffect->operation)
			return FALSE;
		peffect->dec_count(triggering_player);
		core.sub_solving_event.push_back(clit.evt);
		add_process(PROCESSOR_EXECUTE_OPERATION, 0, peffect, 0, triggering_player, 0);
		return FALSE;
	}
	case 3: {
		auto& clit = core.solving_continuous.front();
		effect* peffect = clit.triggering_effect;
		// UNUSED VARIABLE
		// uint8_t triggering_player = clit.triggering_player;
		core.reason_effect = (effect*)core.units.begin()->ptarget;
		core.reason_player = core.units.begin()->arg2;
		if(core.continuous_chain.back().target_cards)
			pduel->delete_group(core.continuous_chain.back().target_cards);
		for(auto& oit : core.continuous_chain.back().opinfos) {
			if(oit.second.op_cards)
				pduel->delete_group(oit.second.op_cards);
		}
		core.continuous_chain.pop_back();
		core.solving_continuous.pop_front();
		if(peffect->is_flag(EFFECT_FLAG_DELAY) || (!(peffect->code & 0x10030000) && (peffect->code & (EVENT_PHASE | EVENT_PHASE_START)))) {
			core.conti_solving = FALSE;
			adjust_all();
			return FALSE;
		}
		return TRUE;
	}
	case 4: {
		if(core.conti_player == PLAYER_NONE)
			core.conti_player = infos.turn_player;
		if(core.conti_player == infos.turn_player) {
			if(core.delayed_continuous_tp.size()) {
				core.sub_solving_continuous.splice(core.sub_solving_continuous.end(), core.delayed_continuous_tp, core.delayed_continuous_tp.begin());
				add_process(PROCESSOR_SOLVE_CONTINUOUS, 0, 0, 0, 0, 0);
			} else
				core.conti_player = 1 - infos.turn_player;
		}
		if(core.conti_player == 1 - infos.turn_player) {
			if(core.delayed_continuous_ntp.size()) {
				core.sub_solving_continuous.splice(core.sub_solving_continuous.end(), core.delayed_continuous_ntp, core.delayed_continuous_ntp.begin());
				add_process(PROCESSOR_SOLVE_CONTINUOUS, 0, 0, 0, 0, 0);
			} else if(core.delayed_continuous_tp.size()) {
				core.conti_player = infos.turn_player;
				core.sub_solving_continuous.splice(core.sub_solving_continuous.end(), core.delayed_continuous_tp, core.delayed_continuous_tp.begin());
				add_process(PROCESSOR_SOLVE_CONTINUOUS, 0, 0, 0, 0, 0);
			} else
				core.conti_player = PLAYER_NONE;
		}
		return TRUE;
	}
	}
	return TRUE;
}
int32_t field::solve_chain(uint16_t step, uint32_t chainend_arg1, uint32_t chainend_arg2) {
	if(core.current_chain.size() == 0 && step == 0)
		return TRUE;
	auto cait = core.current_chain.rbegin();
	switch(step) {
	case 0: {
		if(core.spsummon_rst) {
			if(is_flag(DUEL_CANNOT_SUMMON_OATH_OLD)) {
				set_spsummon_counter(0, false, true);
				set_spsummon_counter(1, false, true);
				core.spsummon_rst = false;
			}
			if(is_flag(DUEL_SPSUMMON_ONCE_OLD_NEGATE)) {
				for(int plr = 0; plr < 2; ++plr) {
					for(auto& iter : core.spsummon_once_map[plr]) {
						auto spcode = iter.first;
						core.spsummon_once_map[plr][spcode] -= core.spsummon_once_map_rst[plr][spcode];
						core.spsummon_once_map_rst[plr][spcode] = 0;
					}
				}
			}
		}
		auto message = pduel->new_message(MSG_CHAIN_SOLVING);
		message->write<uint8_t>(cait->chain_count);
		add_to_disable_check_list(cait->triggering_effect->get_handler());
		adjust_instant();
		raise_event((card*)0, EVENT_CHAIN_ACTIVATING, cait->triggering_effect, 0, cait->triggering_player, cait->triggering_player, cait->chain_count);
		process_instant_event();
		return FALSE;
	}
	case 1: {
		effect* peffect = cait->triggering_effect;
		if(cait->flag & CHAIN_DISABLE_ACTIVATE && is_chain_negatable(cait->chain_count)) {
			remove_oath_effect(peffect);
			if(peffect->is_flag(EFFECT_FLAG_COUNT_LIMIT) && (peffect->count_flag & EFFECT_COUNT_CODE_OATH)) {
				dec_effect_code(peffect->count_code, peffect->count_flag, peffect->count_hopt_index, cait->triggering_player);
			}
			if(cait->applied_chain_counters != nullptr) {
				restore_chain_counter(cait->triggering_player, *cait->applied_chain_counters);
				delete cait->applied_chain_counters;
				cait->applied_chain_counters = nullptr;
			}
			core.new_fchain.remove_if([chaincount = cait->chain_count](const chain& ch) { return ch.evt.event_code == EVENT_CHAINING && ch.evt.event_value == chaincount; });
			core.new_ochain.remove_if([chaincount = cait->chain_count](const chain& ch) { return ch.evt.event_code == EVENT_CHAINING && ch.evt.event_value == chaincount; });
			raise_event((card*)0, EVENT_CHAIN_NEGATED, peffect, 0, cait->triggering_player, cait->triggering_player, cait->chain_count);
			process_instant_event();
			core.units.begin()->step = 9;
			return FALSE;
		}
		if(cait->applied_chain_counters != nullptr) {
			delete cait->applied_chain_counters;
			cait->applied_chain_counters = nullptr;
		}
		release_oath_relation(peffect);
		break_effect();
		core.chain_solving = TRUE;
		raise_event((card*)0, EVENT_CHAIN_SOLVING, peffect, 0, cait->triggering_player, cait->triggering_player, cait->chain_count);
		process_instant_event();
		return FALSE;
	}
	case 2: {
		core.spsummon_state_count_tmp[0] = core.spsummon_state_count[0];
		core.spsummon_state_count_tmp[1] = core.spsummon_state_count[1];
		effect* peffect = cait->triggering_effect;
		card* pcard = peffect->get_handler();
		if((cait->flag & CHAIN_CONTINUOUS_CARD) && !pcard->is_has_relation(*cait)) {
			core.units.begin()->step = 3;
			return FALSE;
		}
		if((peffect->type & EFFECT_TYPE_ACTIVATE) && pcard->is_has_relation(*cait) && !cait->replace_op) {
			pcard->enable_field_effect(true);
			if(is_flag(DUEL_1_FACEUP_FIELD)) {
				if(pcard->data.type & TYPE_FIELD) {
					card* fscard = player[1 - pcard->current.controler].list_szone[5];
					if(fscard && fscard->is_position(POS_FACEUP))
						fscard->enable_field_effect(false);
				}
			}
			adjust_instant();
		}
		// creating continuous target: peffect->is_flag(EFFECT_FLAG_CONTINUOUS_TARGET) && !cait->replace_op
		// operation function creating continuous target should be executed even when disabled
		if(is_chain_disablable(cait->chain_count) && (!peffect->is_flag(EFFECT_FLAG_CONTINUOUS_TARGET) || cait->replace_op)) {
			if(is_chain_disabled(cait->chain_count) || (pcard->get_status(STATUS_DISABLED | STATUS_FORBIDDEN) && pcard->is_has_relation(*cait))) {
				if(!(cait->flag & CHAIN_DISABLE_EFFECT)) {
					auto message = pduel->new_message(MSG_CHAIN_DISABLED);
					message->write<uint8_t>(cait->chain_count);
				}
				raise_event((card*)0, EVENT_CHAIN_DISABLED, peffect, 0, cait->triggering_player, cait->triggering_player, cait->chain_count);
				process_instant_event();
				core.units.begin()->step = 3;
				return FALSE;
			}
		}
		return FALSE;
	}
	case 3 : {
		effect* peffect = cait->triggering_effect;
		card* pcard = peffect->get_handler();
		if((cait->flag & CHAIN_CONTINUOUS_CARD) && !pcard->is_has_relation(*cait)){
			return FALSE;
		}
		if(cait->replace_op) {
			core.units.begin()->arg4 = cait->triggering_effect->operation;
			cait->triggering_effect->operation = cait->replace_op;
		} else
			core.units.begin()->arg4 = 0;
		if(cait->triggering_effect->operation) {
			core.sub_solving_event.push_back(cait->evt);
			add_process(PROCESSOR_EXECUTE_OPERATION, 0, cait->triggering_effect, 0, cait->triggering_player, 0);
		}
		return FALSE;
	}
	case 4: {
		effect* peffect = cait->triggering_effect;
		if(core.units.begin()->arg4) {
			if(peffect->operation != 0)
				luaL_unref(pduel->lua->lua_state, LUA_REGISTRYINDEX, peffect->operation);
			peffect->operation = core.units.begin()->arg4;
			core.units.begin()->arg4 = 0;
		}
		core.special_summoning.clear();
		core.equiping_cards.clear();
		return FALSE;
	}
	case 5: {
		if(core.units.begin()->arg4 == 0) {
			if(cait->opinfos.count(0x200) && cait->opinfos[0x200].op_count) {
				if(is_flag(DUEL_CANNOT_SUMMON_OATH_OLD)) {
					if(core.spsummon_state_count_tmp[cait->triggering_player] == core.spsummon_state_count[cait->triggering_player])
						set_spsummon_counter(cait->triggering_player);
					if(cait->opinfos[0x200].op_player == PLAYER_ALL && core.spsummon_state_count_tmp[1 - cait->triggering_player] == core.spsummon_state_count[1 - cait->triggering_player])
						set_spsummon_counter(1 - cait->triggering_player);
				}
				if(is_flag(DUEL_CANNOT_SUMMON_OATH_OLD)) {
					//sometimes it may add twice, only works for once per turn
					auto& optarget = cait->opinfos[0x200];
					if(optarget.op_cards) {
						if(optarget.op_player == PLAYER_ALL) {
							uint32_t sumplayer = optarget.op_param;
							if(is_flag(DUEL_SPSUMMON_ONCE_OLD_NEGATE) && (core.global_flag & GLOBALFLAG_SPSUMMON_ONCE)) {
								auto opit = optarget.op_cards->container.begin();
								if((*opit)->spsummon_code)
									++core.spsummon_once_map[sumplayer][(*opit)->spsummon_code];
								++opit;
								if((*opit)->spsummon_code)
									++core.spsummon_once_map[1 - sumplayer][(*opit)->spsummon_code];
							}
							auto opit = optarget.op_cards->container.begin();
							check_card_counter(*opit, ACTIVITY_SPSUMMON, sumplayer);
							++opit;
							check_card_counter(*opit, ACTIVITY_SPSUMMON, 1 - sumplayer);
						} else {
							uint32_t sumplayer = cait->triggering_player;
							// genarally setting op_player is unnecessary when the effect targets cards
							// in the case of CATEGORY_SPECIAL_SUMMON(with EFFECT_FLAG_CARD_TARGET), op_player=0x10
							// indecates that it is the opponent that special summons the target monsters
							if(cait->triggering_effect->is_flag(EFFECT_FLAG_CARD_TARGET) && optarget.op_player == 0x10)
								sumplayer = 1 - sumplayer;
							for(auto& ptarget : optarget.op_cards->container) {
								if(is_flag(DUEL_SPSUMMON_ONCE_OLD_NEGATE) && (core.global_flag & GLOBALFLAG_SPSUMMON_ONCE) && ptarget->spsummon_code)
									++core.spsummon_once_map[sumplayer][ptarget->spsummon_code];
								check_card_counter(ptarget, ACTIVITY_SPSUMMON, sumplayer);
							}
						}
					}
				}
			}
		}
		core.spsummon_state_count_tmp[0] = 0;
		core.spsummon_state_count_tmp[1] = 0;
		core.chain_solving = FALSE;
		if(core.delayed_continuous_tp.size()) {
			core.conti_player = infos.turn_player;
			core.sub_solving_continuous.splice(core.sub_solving_continuous.end(), core.delayed_continuous_tp, core.delayed_continuous_tp.begin());
			add_process(PROCESSOR_SOLVE_CONTINUOUS, 0, 0, 0, 0, 0);
		} else if(core.delayed_continuous_ntp.size()) {
			core.conti_player = 1 - infos.turn_player;
			core.sub_solving_continuous.splice(core.sub_solving_continuous.end(), core.delayed_continuous_ntp, core.delayed_continuous_ntp.begin());
			add_process(PROCESSOR_SOLVE_CONTINUOUS, 0, 0, 0, 0, 0);
		} else
			core.conti_player = PLAYER_NONE;
		raise_event((card*)0, EVENT_CHAIN_SOLVED, cait->triggering_effect, 0, cait->triggering_player, cait->triggering_player, cait->chain_count);
		adjust_disable_check_list();
		process_instant_event();
		core.units.begin()->step = 9;
		return FALSE;
	}
	case 10: {
		auto message = pduel->new_message(MSG_CHAIN_SOLVED);
		message->write<uint8_t>(cait->chain_count);
		effect* peffect = cait->triggering_effect;
		card* pcard = peffect->get_handler();
		if((peffect->type & EFFECT_TYPE_ACTIVATE) && (cait->flag & CHAIN_ACTIVATING))
			peffect->type &= ~EFFECT_TYPE_ACTIVATE;
		if((cait->flag & CHAIN_HAND_EFFECT) && !pcard->is_position(POS_FACEUP) && (pcard->current.location == LOCATION_HAND))
			shuffle(pcard->current.controler, LOCATION_HAND);
		if(cait->target_cards && cait->target_cards->container.size()) {
			for(auto& ptarget : cait->target_cards->container)
				ptarget->release_relation(*cait);
		}
		if((pcard->data.type & TYPE_EQUIP) && (peffect->type & EFFECT_TYPE_ACTIVATE)
			&& !pcard->equiping_target && pcard->is_has_relation(*cait))
			pcard->set_status(STATUS_LEAVE_CONFIRMED, TRUE);
		if(is_flag(DUEL_1_FACEUP_FIELD)) {
			if((pcard->data.type & TYPE_FIELD) && (peffect->type & EFFECT_TYPE_ACTIVATE)
					&& !pcard->is_status(STATUS_LEAVE_CONFIRMED) && pcard->is_has_relation(*cait)) {
				card* fscard = player[1 - pcard->current.controler].list_szone[5];
				if(fscard && fscard->is_position(POS_FACEUP))
					destroy(fscard, 0, REASON_RULE, 1 - pcard->current.controler);
			}
		}
		peffect->active_type = 0;
		peffect->active_handler = 0;
		pcard->release_relation(*cait);
		if(cait->target_cards)
			pduel->delete_group(cait->target_cards);
		for(auto& oit : cait->opinfos) {
			if(oit.second.op_cards)
				pduel->delete_group(oit.second.op_cards);
		}
		for(auto& cit : core.delayed_enable_set) {
			if(cit->current.location == LOCATION_MZONE)
				cit->enable_field_effect(true);
		}
		core.delayed_enable_set.clear();
		adjust_all();
		core.current_chain.pop_back();
		if(--core.real_chain_count < 0)
			core.real_chain_count = 0;
		if(!core.current_chain.size()) {
			for(auto& ch_lim : core.chain_limit)
				luaL_unref(pduel->lua->lua_state, LUA_REGISTRYINDEX, ch_lim.function);
			core.chain_limit.clear();
			return FALSE;
		}
		if(core.summoning_card || core.summoning_proc_group_type)
			core.subunits.push_back(core.reserved);
		core.summoning_card = 0;
		core.summoning_proc_group_type = 0;
		core.units.begin()->step = -1;
		return FALSE;
	}
	case 11: {
		for(auto cit = core.leave_confirmed.begin(); cit != core.leave_confirmed.end();) {
			auto rm = cit++;
			if(!(*rm)->is_status(STATUS_LEAVE_CONFIRMED))
				core.leave_confirmed.erase(rm);
		}
		if(core.leave_confirmed.size())
			send_to(core.leave_confirmed, 0, REASON_RULE, PLAYER_NONE, PLAYER_NONE, LOCATION_GRAVE, 0, POS_FACEUP);
		return FALSE;
	}
	case 12: {
		core.used_event.splice(core.used_event.end(), core.point_event);
		pduel->new_message(MSG_CHAIN_END);
		for(auto& ch_lim_p : core.chain_limit_p)
			luaL_unref(pduel->lua->lua_state, LUA_REGISTRYINDEX, ch_lim_p.function);
		core.chain_limit_p.clear();
		reset_chain();
		if(core.summoning_card || core.summoning_proc_group_type || core.effect_damage_step == 1)
			core.subunits.push_back(core.reserved);
		core.summoning_proc_group_type = 0;
		core.summoning_card = nullptr;
		return FALSE;
	}
	case 13: {
		core.just_sent_cards.clear();
		raise_event((card*)0, EVENT_CHAIN_END, 0, 0, 0, 0, 0);
		process_instant_event();
		adjust_all();
		if(chainend_arg1 != 0x101 || chainend_arg2 != TRUE) {
			core.hint_timing[0] |= TIMING_CHAIN_END;
			core.hint_timing[1] |= TIMING_CHAIN_END;
			add_process(PROCESSOR_POINT_EVENT, 0, 0, 0, chainend_arg1, chainend_arg2);
		}
		returns.set<int32_t>(0, TRUE);
		return TRUE;
	}
	}
	return TRUE;
}
int32_t field::break_effect(bool clear_sent) {
	if(clear_sent)
		core.just_sent_cards.clear();
	core.hint_timing[0] &= TIMING_DAMAGE_STEP | TIMING_DAMAGE_CAL;
	core.hint_timing[1] &= TIMING_DAMAGE_STEP | TIMING_DAMAGE_CAL;
	for (auto chit = core.new_ochain.begin(); chit != core.new_ochain.end();) {
		auto rm = chit++;
		effect* peffect = rm->triggering_effect;
		if (!peffect->is_flag(EFFECT_FLAG_DELAY)) {
			if (peffect->is_flag(EFFECT_FLAG_FIELD_ONLY)
			        || !(peffect->type & EFFECT_TYPE_FIELD) || peffect->in_range(*rm)) {
				auto message = pduel->new_message(MSG_MISSED_EFFECT);
				message->write(peffect->get_handler()->get_info_location());
				message->write<uint32_t>(peffect->get_handler()->data.code);
			}
			core.new_ochain.erase(rm);
		}
	}
	core.used_event.splice(core.used_event.end(), core.instant_event);
	adjust_instant();
	if(!is_flag(DUEL_RELAY) && !core.force_turn_end) {
		uint32_t winp = 5, rea = 1;
		if(player[0].lp <= 0 && player[1].lp > 0 && !is_player_affected_by_effect(0, EFFECT_CANNOT_LOSE_LP)) {
			winp = 1;
			rea = 1;
		}
		if(core.overdraw[0] && !core.overdraw[1] && !is_player_affected_by_effect(0, EFFECT_CANNOT_LOSE_DECK)) {
			winp = 1;
			rea = 2;
		}
		if(player[1].lp <= 0 && player[0].lp > 0 && !is_player_affected_by_effect(1, EFFECT_CANNOT_LOSE_LP)) {
			winp = 0;
			rea = 1;
		}
		if(core.overdraw[1] && !core.overdraw[0] && !is_player_affected_by_effect(1, EFFECT_CANNOT_LOSE_DECK)) {
			winp = 0;
			rea = 2;
		}
		if(player[1].lp <= 0 && player[0].lp <= 0 && !(is_player_affected_by_effect(0, EFFECT_CANNOT_LOSE_LP) && is_player_affected_by_effect(1, EFFECT_CANNOT_LOSE_LP))) {
			if(is_player_affected_by_effect(0, EFFECT_CANNOT_LOSE_LP))
				winp = 0;
			else if(is_player_affected_by_effect(1, EFFECT_CANNOT_LOSE_LP))
				winp = 1;
			else
				winp = PLAYER_NONE;
			rea = 1;
		}
		if(core.overdraw[1] && core.overdraw[0] && !(is_player_affected_by_effect(0, EFFECT_CANNOT_LOSE_DECK) && is_player_affected_by_effect(1, EFFECT_CANNOT_LOSE_DECK))) {
			if(is_player_affected_by_effect(0, EFFECT_CANNOT_LOSE_DECK))
				winp = 0;
			else if(is_player_affected_by_effect(1, EFFECT_CANNOT_LOSE_DECK))
				winp = 1;
			else
				winp = PLAYER_NONE;
			rea = 2;
		}
		if(winp != 5) {
			auto message = pduel->new_message(MSG_WIN);
			message->write<uint8_t>(winp);
			message->write<uint8_t>(rea);
			core.overdraw[0] = core.overdraw[1] = FALSE;
			core.win_player = 5;
			core.win_reason = 0;
		} else if(core.win_player != 5) {
			auto message = pduel->new_message(MSG_WIN);
			message->write<uint8_t>(core.win_player);
			message->write<uint8_t>(core.win_reason);
			core.win_player = 5;
			core.win_reason = 0;
			core.overdraw[0] = core.overdraw[1] = FALSE;
		}
	}
	return 0;
}
void field::adjust_instant() {
	++infos.event_id;
	adjust_disable_check_list();
	adjust_self_destroy_set();
}
void field::adjust_all() {
	++infos.event_id;
	core.readjust_map.clear();
	add_process(PROCESSOR_ADJUST, 0, 0, 0, 0, 0);
}
void field::refresh_location_info_instant() {
	effect_set eset;
	uint32_t dis1 = player[0].disabled_location | (player[1].disabled_location << 16);
	player[0].disabled_location = 0;
	player[1].disabled_location = 0;
	filter_field_effect(EFFECT_DISABLE_FIELD, &eset);
	for(const auto& peff : eset) {
		uint32_t value = peff->get_value();
		player[0].disabled_location |= value & 0xff7f;
		player[1].disabled_location |= (value >> 16) & 0xff7f;
	}
	eset.clear();
	filter_field_effect(EFFECT_USE_EXTRA_MZONE, &eset);
	for(const auto& peff : eset) {
		uint32_t p = peff->get_handler_player();
		uint32_t value = peff->get_value();
		player[p].disabled_location |= (value >> 16) & 0x1f;
	}
	eset.clear();
	filter_field_effect(EFFECT_USE_EXTRA_SZONE, &eset);
	for(const auto& peff : eset) {
		uint32_t p = peff->get_handler_player();
		uint32_t value = peff->get_value();
		player[p].disabled_location |= (value >> 8) & 0x1f00;
	}
	player[0].disabled_location |= (((player[1].disabled_location >> 5) & 1) << 6) | (((player[1].disabled_location >> 6) & 1) << 5);
	player[1].disabled_location |= (((player[0].disabled_location >> 5) & 1) << 6) | (((player[0].disabled_location >> 6) & 1) << 5);
	uint32_t dis2 = player[0].disabled_location | (player[1].disabled_location << 16);
	if(dis1 != dis2) {
		auto message = pduel->new_message(MSG_FIELD_DISABLED);
		message->write<uint32_t>(dis2);
	}
}
int32_t field::refresh_location_info(uint16_t step) {
	switch(step) {
	case 0: {
		effect_set eset;
		if(is_flag(DUEL_3_COLUMNS_FIELD)) {
			player[0].used_location |= 0x1111;
			player[1].used_location |= 0x1111;
		}
		core.units.begin()->arg2 = player[0].disabled_location | (player[1].disabled_location << 16);
		player[0].disabled_location = 0;
		player[1].disabled_location = 0;
		core.disfield_effects.clear();
		core.extra_mzone_effects.clear();
		core.extra_szone_effects.clear();
		filter_field_effect(EFFECT_DISABLE_FIELD, &eset);
		for(const auto& peff : eset) {
			uint32_t value = peff->get_value();
			if(value && !peff->is_flag(EFFECT_FLAG_REPEAT)) {
				player[0].disabled_location |= value & 0xff7f;
				player[1].disabled_location |= (value >> 16) & 0xff7f;
			} else
				core.disfield_effects.push_back(peff);
		}
		eset.clear();
		filter_field_effect(EFFECT_USE_EXTRA_MZONE, &eset);
		for(const auto& peff : eset) {
			uint32_t p = peff->get_handler_player();
			uint32_t value = peff->get_value();
			player[p].disabled_location |= (value >> 16) & 0x1f;
			if((uint32_t)field_used_count[(value >> 16) & 0x1f] < (value & 0xffff))
				core.extra_mzone_effects.push_back(peff);
		}
		eset.clear();
		filter_field_effect(EFFECT_USE_EXTRA_SZONE, &eset);
		for(const auto& peff : eset) {
			uint32_t p = peff->get_handler_player();
			uint32_t value = peff->get_value();
			player[p].disabled_location |= (value >> 8) & 0x1f00;
			if((uint32_t)field_used_count[(value >> 16) & 0x1f] < (value & 0xffff))
				core.extra_szone_effects.push_back(peff);
		}
		return FALSE;
	}
	case 1: {
		if(core.disfield_effects.size() == 0) {
			core.units.begin()->step = 2;
			return FALSE;
		}
		effect* peffect = core.disfield_effects[0];
		core.units.begin()->peffect = peffect;
		core.disfield_effects.erase(core.disfield_effects.begin());
		if(!peffect->operation) {
			peffect->value = 0x80;
			core.units.begin()->step = 0;
			return FALSE;
		}
		core.sub_solving_event.push_back(nil_event);
		add_process(PROCESSOR_EXECUTE_OPERATION, 0, peffect, 0, peffect->get_handler_player(), 0);
		return FALSE;
	}
	case 2: {
		auto disabled_locations = returns.at<uint32_t>(0);
		disabled_locations &= 0xff7fff7f;
		if(disabled_locations == 0)
			disabled_locations = 0x80;
		if(core.units.begin()->peffect->get_handler_player() == 0) {
			core.units.begin()->peffect->value = disabled_locations;
			player[0].disabled_location |= disabled_locations & 0xff7f;
			player[1].disabled_location |= (disabled_locations >> 16) & 0xff7f;
		} else {
			core.units.begin()->peffect->value = ((disabled_locations << 16) | (disabled_locations >> 16));
			player[1].disabled_location |= disabled_locations & 0xff7f;
			player[0].disabled_location |= (disabled_locations >> 16) & 0xff7f;
		}
		returns.set<uint32_t>(0, disabled_locations);
		core.units.begin()->step = 0;
		return FALSE;
	}
	case 3: {
		if(core.extra_mzone_effects.size() == 0) {
			core.units.begin()->step = 4;
			return FALSE;
		}
		effect* peffect = core.extra_mzone_effects[0];
		core.units.begin()->peffect = peffect;
		core.extra_mzone_effects.erase(core.extra_mzone_effects.begin());
		uint32_t p = peffect->get_handler_player();
		uint32_t mzone_flag = (player[p].disabled_location | player[p].used_location) & 0x1f;
		if(mzone_flag == 0x1f) {
			core.units.begin()->step = 4;
			return FALSE;
		}
		int32_t val = peffect->get_value();
		int32_t dis_count = (val & 0xffff) - field_used_count[(val >> 16) & 0x1f];
		int32_t empty_count = 5 - field_used_count[mzone_flag];
		uint32_t flag = mzone_flag | 0xffffffe0;
		if(dis_count > empty_count)
			dis_count = empty_count;
		core.units.begin()->arg1 = dis_count;
		add_process(PROCESSOR_SELECT_DISFIELD, 0, 0, 0, p, flag, dis_count);
		return FALSE;
	}
	case 4: {
		uint32_t dis_count = core.units.begin()->arg1;
		uint32_t mzone_flag = 0;
		uint8_t pt = 0;
		for(uint32_t i = 0; i < dis_count; ++i) {
			uint8_t s = returns.at<int8_t>(pt + 2);
			mzone_flag |= 0x1u << s;
			pt += 3;
		}
		effect* peffect = core.units.begin()->peffect;
		player[peffect->get_handler_player()].disabled_location |= mzone_flag;
		peffect->value = (int32_t)(peffect->value | (mzone_flag << 16));
		core.units.begin()->step = 2;
		return FALSE;
	}
	case 5: {
		if(core.extra_szone_effects.size() == 0) {
			core.units.begin()->step = 6;
			return FALSE;
		}
		effect* peffect = core.extra_szone_effects[0];
		core.units.begin()->peffect = peffect;
		core.extra_szone_effects.erase(core.extra_szone_effects.begin());
		uint32_t p = peffect->get_handler_player();
		uint32_t szone_flag = ((player[p].disabled_location | player[p].used_location) >> 8) & 0x1f;
		if(szone_flag == 0x1f) {
			core.units.begin()->step = 6;
			return FALSE;
		}
		int32_t val = peffect->get_value();
		uint32_t dis_count = (val & 0xffff) - field_used_count[(val >> 16) & 0x1f];
		uint32_t empty_count = 5 - field_used_count[szone_flag];
		uint32_t flag = (szone_flag << 8) | 0xffffe0ff;
		if(dis_count > empty_count)
			dis_count = empty_count;
		core.units.begin()->arg1 = dis_count;
		add_process(PROCESSOR_SELECT_DISFIELD, 0, 0, 0, p, flag, dis_count);
		return FALSE;
	}
	case 6: {
		uint32_t dis_count = core.units.begin()->arg1;
		uint32_t szone_flag = 0;
		uint8_t pt = 0;
		for(uint32_t i = 0; i < dis_count; ++i) {
			uint8_t s = returns.at<int8_t>(pt + 2);
			szone_flag |= 0x1u << s;
			pt += 3;
		}
		effect* peffect = core.units.begin()->peffect;
		player[peffect->get_handler_player()].disabled_location |= szone_flag << 8;
		peffect->value = (int32_t)(peffect->value | (szone_flag << 16));
		core.units.begin()->step = 4;
		return FALSE;
	}
	case 7: {
		player[0].disabled_location |= (((player[1].disabled_location >> 5) & 1) << 6) | (((player[1].disabled_location >> 6) & 1) << 5);
		player[1].disabled_location |= (((player[0].disabled_location >> 5) & 1) << 6) | (((player[0].disabled_location >> 6) & 1) << 5);
		uint32_t dis = player[0].disabled_location | (player[1].disabled_location << 16);
		if(dis != (uint32_t)core.units.begin()->arg2) {
			auto message = pduel->new_message(MSG_FIELD_DISABLED);
			message->write<uint32_t>(dis);
		}
		return TRUE;
	}
	}
	return TRUE;
}
int32_t field::adjust_step(uint16_t step) {
	switch(step) {
	case 0: {
		core.re_adjust = FALSE;
		return FALSE;
	}
	case 1: {
		//win check(deck=0 or lp=0)
		if(!core.force_turn_end){
			uint32_t winp = 5, rea = 1;
			if (player[0].lp <= 0 && player[1].lp > 0 && !is_player_affected_by_effect(0, EFFECT_CANNOT_LOSE_LP)) {
				winp = 1;
				rea = 1;
			}
			if (core.overdraw[0] && !core.overdraw[1] && !is_player_affected_by_effect(0, EFFECT_CANNOT_LOSE_DECK)) {
				winp = 1;
				rea = 2;
			}
			if (player[1].lp <= 0 && player[0].lp > 0 && !is_player_affected_by_effect(1, EFFECT_CANNOT_LOSE_LP)) {
				winp = 0;
				rea = 1;
			}
			if (core.overdraw[1] && !core.overdraw[0] && !is_player_affected_by_effect(1, EFFECT_CANNOT_LOSE_DECK)) {
				winp = 0;
				rea = 2;
			}
			if (player[1].lp <= 0 && player[0].lp <= 0 && !(is_player_affected_by_effect(0, EFFECT_CANNOT_LOSE_LP) && is_player_affected_by_effect(1, EFFECT_CANNOT_LOSE_LP))) {
				if (is_player_affected_by_effect(0, EFFECT_CANNOT_LOSE_LP))
					winp = 0;
				else if (is_player_affected_by_effect(1, EFFECT_CANNOT_LOSE_LP))
					winp = 1;
				else
					winp = PLAYER_NONE;
				rea = 1;
			}
			if (core.overdraw[1] && core.overdraw[0] && !(is_player_affected_by_effect(0, EFFECT_CANNOT_LOSE_DECK) && is_player_affected_by_effect(1, EFFECT_CANNOT_LOSE_DECK))) {
				if (is_player_affected_by_effect(0, EFFECT_CANNOT_LOSE_DECK))
					winp = 0;
				else if (is_player_affected_by_effect(1, EFFECT_CANNOT_LOSE_DECK))
					winp = 1;
				else
					winp = PLAYER_NONE;
				rea = 2;
			}
			if (is_flag(DUEL_RELAY)) {
				if (winp == PLAYER_NONE) {
					bool p1 = relay_check(0);
					bool p2 = relay_check(1);
					if (p1 && !p2)
						winp = 0;
					else if (!p1 && p2)
						winp = 1;
					else if (p1 && p2) {
						winp = 5;
						core.overdraw[0] = core.overdraw[1] = FALSE;
					}
				} else if (winp < PLAYER_NONE)
					if (relay_check(1 - winp)) {
						winp = 5;
						core.overdraw[0] = core.overdraw[1] = FALSE;
					}
			}
			if(winp != 5) {
				auto message = pduel->new_message(MSG_WIN);
				message->write<uint8_t>(winp);
				message->write<uint8_t>(rea);
				core.overdraw[0] = core.overdraw[1] = FALSE;
				core.win_player = 5;
				core.win_reason = 0;
			} else if(core.win_player != 5) {
				auto message = pduel->new_message(MSG_WIN);
				message->write<uint8_t>(core.win_player);
				message->write<uint8_t>(core.win_reason);
				core.win_player = 5;
				core.win_reason = 0;
				core.overdraw[0] = core.overdraw[1] = FALSE;
			}
		}
		return FALSE;
	}
	case 2: {
		//disable check
		uint8_t tp = infos.turn_player;
		for(uint8_t p = 0; p < 2; ++p) {
			for(auto& pcard : player[tp].list_mzone) {
				if(pcard)
					add_to_disable_check_list(pcard);
			}
			for(auto& pcard : player[tp].list_szone) {
				if(pcard)
					add_to_disable_check_list(pcard);
			}
			tp = 1 - tp;
		}
		adjust_disable_check_list();
		add_process(PROCESSOR_REFRESH_LOC, 0, 0, 0, 0, 0);
		return FALSE;
	}
	case 3: {
		//trap monster
		core.trap_monster_adjust_set[0].clear();
		core.trap_monster_adjust_set[1].clear();
		for(uint8_t p = 0; p < 2; ++p) {
			for(auto& pcard : player[p].list_mzone) {
				if(!pcard) continue;
				if((pcard->get_type() & TYPE_TRAPMONSTER) && pcard->is_affected_by_effect(EFFECT_DISABLE_TRAPMONSTER)) {
					core.trap_monster_adjust_set[p].insert(pcard);
				}
			}
		}
		if(core.trap_monster_adjust_set[0].size() || core.trap_monster_adjust_set[1].size()) {
			core.re_adjust = TRUE;
			add_process(PROCESSOR_TRAP_MONSTER_ADJUST, 0, 0, 0, 0, 0);
		}
		return FALSE;
	}
	case 4: {
		//control
		core.control_adjust_set[0].clear();
		core.control_adjust_set[1].clear();
		card_set reason_cards;
		for(uint8_t p = 0; p < 2; ++p) {
			for(auto& pcard : player[p].list_mzone) {
				if(!pcard) continue;
				uint8_t cur = pcard->current.controler;
				auto res = pcard->refresh_control_status();
				uint8_t ref = std::get<uint8_t>(res);
				effect* peffect = std::get<effect*>(res);
				if(cur != ref && pcard->is_capable_change_control()) {
					core.control_adjust_set[p].insert(pcard);
					if(peffect && (!(peffect->type & EFFECT_TYPE_SINGLE) || peffect->condition))
						reason_cards.insert(peffect->get_handler());
				}
			}
		}
		if(core.control_adjust_set[0].size() || core.control_adjust_set[1].size()) {
			core.re_adjust = TRUE;
			get_control(core.control_adjust_set[1 - infos.turn_player], 0, PLAYER_NONE, infos.turn_player, 0, 0, 0xff);
			get_control(core.control_adjust_set[infos.turn_player], 0, PLAYER_NONE, 1 - infos.turn_player, 0, 0, 0xff);
			for(auto& rcard : reason_cards) {
				++core.readjust_map[rcard];
				if(core.readjust_map[rcard] > 3)
					destroy(rcard, 0, REASON_RULE, PLAYER_NONE);
			}
		}
		core.last_control_changed_id = infos.field_id;
		return FALSE;
	}
	case 5: {
		//remove brainwashing
		if(core.global_flag & GLOBALFLAG_BRAINWASHING_CHECK) {
			core.control_adjust_set[0].clear();
			core.control_adjust_set[1].clear();
			effect_set eset;
			filter_field_effect(EFFECT_REMOVE_BRAINWASHING, &eset, FALSE);
			uint32_t res = eset.size() ? TRUE : FALSE;
			if(res) {
				for(uint8_t p = 0; p < 2; ++p) {
					for(auto& pcard : player[p].list_mzone) {
						if(pcard && pcard->is_affected_by_effect(EFFECT_REMOVE_BRAINWASHING)) {
							//the opposite of pcard->check_control_effect()
							auto pr = pcard->single_effect.equal_range(EFFECT_SET_CONTROL);
							for(auto eit = pr.first; eit != pr.second;) {
								effect* peffect = eit->second;
								++eit;
								if(!peffect->condition)
									peffect->handler->remove_effect(peffect);
							}
							if(p != pcard->owner && pcard->is_capable_change_control())
								core.control_adjust_set[p].insert(pcard);
						}
					}
				}
			}
			core.remove_brainwashing = res;
			if(core.control_adjust_set[0].size() || core.control_adjust_set[1].size()) {
				core.re_adjust = TRUE;
				get_control(core.control_adjust_set[1 - infos.turn_player], 0, PLAYER_NONE, infos.turn_player, 0, 0, 0xff);
				get_control(core.control_adjust_set[infos.turn_player], 0, PLAYER_NONE, 1 - infos.turn_player, 0, 0, 0xff);
			}
		}
		core.units.begin()->step = 7;
		return FALSE;
	}
	case 8: {
		if(adjust_grant_effect())
			core.re_adjust = TRUE;
		return FALSE;
	}
	case 9: {
		if(core.selfdes_disabled) {
			core.units.begin()->step = 10;
			return FALSE;
		}
		//self destroy
		adjust_self_destroy_set();
		return FALSE;
	}
	case 10: {
		//equip check
		uint8_t tp = infos.turn_player;
		card_set destroy_set;
		for(uint8_t p = 0; p < 2; ++p) {
			for(uint8_t i = 0; i < 5; ++i) {
				card* pcard = player[tp].list_szone[i];
				if(pcard && pcard->equiping_target && !pcard->is_affected_by_effect(EFFECT_EQUIP_LIMIT, pcard->equiping_target))
					destroy_set.insert(pcard);
			}
			tp = 1 - tp;
		}
		if(destroy_set.size()) {
			core.re_adjust = TRUE;
			destroy(std::move(destroy_set), 0, REASON_RULE, PLAYER_NONE);
		}
		return FALSE;
	}
	case 11: {
		//position
		uint32_t tp = infos.turn_player, pos;
		card_set pos_adjust;
		effect_set eset;
		for(uint8_t p = 0; p < 2; ++p) {
			for(auto& pcard : player[tp].list_mzone) {
				if(!pcard || ((pcard->data.type & TYPE_LINK) && (pcard->data.type & TYPE_MONSTER)) || pcard->is_affected_by_effect(EFFECT_CANNOT_CHANGE_POS_E))
					continue;
				eset.clear();
				pcard->filter_effect(EFFECT_SET_POSITION, &eset);
				if(eset.size()) {
					pos = eset.back()->get_value();
					if((pos & 0xff) != pcard->current.position) {
						pos_adjust.insert(pcard);
						pcard->position_param = pos;
						if(pcard->is_status(STATUS_JUST_POS))
							pcard->set_status(STATUS_CONTINUOUS_POS, TRUE);
						else
							pcard->set_status(STATUS_CONTINUOUS_POS, FALSE);
					} else
						pcard->set_status(STATUS_CONTINUOUS_POS, FALSE);
					pcard->set_status(STATUS_JUST_POS, FALSE);
				}
			}
			tp = 1 - tp;
		}
		if(pos_adjust.size()) {
			core.re_adjust = TRUE;
			group* ng = pduel->new_group();
			ng->container.swap(pos_adjust);
			ng->is_readonly = TRUE;
			add_process(PROCESSOR_CHANGEPOS, 0, 0, ng, PLAYER_NONE, TRUE);
		}
		return FALSE;
	}
	case 12: {
		//shuffle check
		for(auto& pcard : player[0].list_hand) {
			effect* pub = pcard->is_affected_by_effect(EFFECT_PUBLIC);
			if(!pub && pcard->is_position(POS_FACEUP))
				core.shuffle_hand_check[0] = TRUE;
			pcard->current.position = pub ? POS_FACEUP : POS_FACEDOWN;
		}
		for(auto& pcard : player[1].list_hand) {
			effect* pub = pcard->is_affected_by_effect(EFFECT_PUBLIC);
			if(!pub && pcard->is_position(POS_FACEUP))
				core.shuffle_hand_check[1] = TRUE;
			pcard->current.position = pub ? POS_FACEUP : POS_FACEDOWN;
		}
		if(core.shuffle_hand_check[infos.turn_player])
			shuffle(infos.turn_player, LOCATION_HAND);
		if(core.shuffle_hand_check[1 - infos.turn_player])
			shuffle(1 - infos.turn_player, LOCATION_HAND);
		return FALSE;
	}
	case 13: {
		//reverse_deck
		effect_set eset;
		uint32_t res = 0;
		if(core.global_flag & GLOBALFLAG_DECK_REVERSE_CHECK) {
			filter_field_effect(EFFECT_REVERSE_DECK, &eset, FALSE);
			res = eset.size() ? TRUE : FALSE;
			if(core.deck_reversed ^ res) {
				reverse_deck(0);
				reverse_deck(1);
				pduel->new_message(MSG_REVERSE_DECK);
				if(res) {
					if(player[0].list_main.size()) {
						card* ptop = player[0].list_main.back();
						auto message = pduel->new_message(MSG_DECK_TOP);
						message->write<uint8_t>(0);
						message->write<uint32_t>(0);
						message->write<uint32_t>(ptop->data.code);
						message->write<uint32_t>(ptop->current.position);
					}
					if(player[1].list_main.size()) {
						card* ptop = player[1].list_main.back();
						auto message = pduel->new_message(MSG_DECK_TOP);
						message->write<uint8_t>(1);
						message->write<uint32_t>(0);
						message->write<uint32_t>(ptop->data.code);
						message->write<uint32_t>(ptop->current.position);
					}
				}
			}
			core.deck_reversed = res;
		}
		return FALSE;
	}
	case 14: {
		//attack cancel
		card* attacker = core.attacker;
		if(!attacker)
			return FALSE;
		if(!attacker->is_affected_by_effect(EFFECT_UNSTOPPABLE_ATTACK)) {
			if(attacker->is_status(STATUS_ATTACK_CANCELED))
				return FALSE;
		}
		if(infos.phase != PHASE_DAMAGE && infos.phase != PHASE_DAMAGE_CAL) {
			if(!core.attacker->is_capable_attack()
				|| core.attacker->current.controler != core.attacker->attack_controler
				|| core.attacker->fieldid_r != core.pre_field[0]) {
				attacker->set_status(STATUS_ATTACK_CANCELED, TRUE);
				return FALSE;
			}
			if(core.attack_rollback)
				return FALSE;
			std::set<uint32_t> fidset;
			for(auto& pcard : player[1 - infos.turn_player].list_mzone) {
				if(pcard)
					fidset.insert(pcard->fieldid_r);
			}
			if(fidset != core.opp_mzone || !confirm_attack_target())
				core.attack_rollback = TRUE;
		} else {
			if(core.attacker->current.location != LOCATION_MZONE || core.attacker->fieldid_r != core.pre_field[0]
				|| ((core.attacker->current.position & POS_DEFENSE) && !(core.attacker->is_affected_by_effect(EFFECT_DEFENSE_ATTACK)))
				|| core.attacker->current.controler != core.attacker->attack_controler
				|| (core.attack_target && (core.attack_target->current.location != LOCATION_MZONE
					|| core.attack_target->current.controler != core.attack_target->attack_controler
					|| core.attack_target->fieldid_r != core.pre_field[1])))
				core.attacker->set_status(STATUS_ATTACK_CANCELED, TRUE);
		}
		return FALSE;
	}
	case 15: {
		raise_event((card*)0, EVENT_ADJUST, 0, 0, PLAYER_NONE, PLAYER_NONE, 0);
		process_instant_event();
		return FALSE;
	}
	case 16: {
		if(core.re_adjust) {
			core.units.begin()->step = -1;
			return FALSE;
		}
		if(core.shuffle_hand_check[0])
			shuffle(0, LOCATION_HAND);
		if(core.shuffle_hand_check[1])
			shuffle(1, LOCATION_HAND);
		if(core.shuffle_deck_check[0])
			shuffle(0, LOCATION_DECK);
		if(core.shuffle_deck_check[1])
			shuffle(1, LOCATION_DECK);
		return TRUE;
	}
	}
	return TRUE;
}

int32_t field::startup(uint16_t step) {
	switch(step) {
	case 0: {
		core.shuffle_hand_check[0] = FALSE;
		core.shuffle_hand_check[1] = FALSE;
		core.shuffle_deck_check[0] = FALSE;
		core.shuffle_deck_check[1] = FALSE;
		raise_event((card*)0, EVENT_STARTUP, 0, 0, 0, 0, 0);
		process_instant_event();
		return FALSE;
	}
	case 1: {
		for(int p = 0; p < 2; p++) {
			core.shuffle_hand_check[p] = FALSE;
			core.shuffle_deck_check[p] = FALSE;
			if(player[p].start_count > 0)
				draw(0, REASON_RULE, PLAYER_NONE, p, player[p].start_count);
			auto list_size = player[p].extra_lists_main.size();
			for(size_t l = 0; l < list_size; l++) {
				auto& main = player[p].extra_lists_main[l];
				auto& hand = player[p].extra_lists_hand[l];
				for(int i = 0; i < player[p].start_count && !main.empty(); ++i) {
					card* pcard = main.back();
					main.pop_back();
					hand.push_back(pcard);
					pcard->current.controler = p;
					pcard->current.location = LOCATION_HAND;
					pcard->current.sequence = static_cast<uint32_t>(hand.size() - 1);
					pcard->current.position = POS_FACEDOWN;
				}

			}
		}
		add_process(PROCESSOR_TURN, 0, 0, 0, 0, 0);
		return TRUE;
	}
	}
	return TRUE;
}

int32_t field::refresh_relay(uint16_t step) {
	switch(step) {
	case 0:
	case 1:
		if(player[step].recharge)
			next_player(step);
		return FALSE;
	default:
		return TRUE;
	}
}
