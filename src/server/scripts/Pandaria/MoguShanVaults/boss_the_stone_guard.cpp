/* # Script de Sungis : The Stone Guard # */

/* Notes : What is missing ? :	- Guardians personnal spells
								- Overloads with power
								- Anim sit fails after evade
*/

#include "ScriptPCH.h"
#include "mogushan_vaults.h"

enum Spells
{
	/* Amethyst Guardian */
    SPELL_AMETHYST_OVERLOAD				= 115844,
    SPELL_AMETHYST_PETRIFICATION		= 116057,
    SPELL_AMETHYST_PETRIFICATION_BAR	= 131255,
    SPELL_AMETHYST_TRUE_FORM			= 115829,
    SPELL_AMETHYST_POOL					= 116235,

	/* Cobalt Guardian */
    SPELL_COBALT_OVERLOAD				= 115840,
    SPELL_COBALT_PETRIFICATION			= 115852,
    SPELL_COBALT_PETRIFICATION_BAR		= 131268,
    SEPLL_COBALT_TRUE_FORM				= 115771,
    SPELL_COBALT_MINE					= 129460,

	/* Jade Guardian */
    SPELL_JADE_OVERLOAD                 = 115842,
    SPELL_JADE_PETRIFICATION            = 116006,
    SPELL_JADE_PETRIFICATION_BAR        = 131269,
    SEPLL_JADE_TRUE_FORM                = 115827,
    SPELL_JADE_SHARDS                   = 116223,

	/* Jasper Guardian */
    SPELL_JASPER_OVERLOAD				= 115843,
    SPELL_JASPER_PETRIFICATION			= 116036,
    SPELL_JASPER_PETRIFICATION_BAR		= 131270,
    SEPLL_JASPER_TRUE_FORM				= 115828,
    SPELL_JASPER_CHAINS					= 130395,
    SPELL_JASPER_CHAINS_VISUAL			= 130403,
    SPELL_JASPER_CHAINS_DAMAGE			= 130404,

    /* Shared Guardian Spells */
    SPELL_SOLID_STONE					= 115745,
	SPELL_STONE_VISUAL					= 123947,
	SPELL_ANIM_SIT						= 128886,
    SPELL_REND_FLESH					= 125206,
    SPELL_ZERO_POWER					= 118357,
    SPELL_TOTALY_PETRIFIED				= 115877,
	SPELL_BERSERK						= 26662,
};

enum Events
{
	/* Shared Guardian Events */
	EVENT_REND_FLESH				= 1,
	EVENT_PETRIFICATION_FIRST		= 2,
	EVENT_PETRIFICATION_SET			= 3,
	EVENT_PETRIFICATION_INCREASE_1	= 4,
	EVENT_PETRIFICATION_INCREASE_2	= 5,
	EVENT_PETRIFICATION_INCREASE_3	= 6,
	EVENT_INCREASE_POWER_1			= 7,
	EVENT_INCREASE_POWER_2			= 8,

	/* The Stone Guard Tracker */
	EVENT_CHOOSE_PETRIFICATION		= 1,
};

enum Actions
{
	ACTION_CHOOSE_PETRIFICATION,
	ACTION_PETRIFICATION_BAR,
};

enum Npcs
{
	NPC_THE_STONE_GUARD_TRACKER		= 400463,
};

class boss_amethyst_guardian : public CreatureScript
{
    public:
        boss_amethyst_guardian() : CreatureScript("boss_amethyst_guardian") { }

		CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_amethyst_guardianAI(creature);
        }

        struct boss_amethyst_guardianAI : public ScriptedAI
        {
            boss_amethyst_guardianAI(Creature* creature) : ScriptedAI(creature)
            {
				instance = creature->GetInstanceScript();
            }

			InstanceScript* instance;
			EventMap events;
			Map* map;
			bool solidStone;

            void Reset()
            {
				events.Reset();
				solidStone = false;
				map = me->GetMap();
				
				if (uint32 vehicleId = me->GetVehicleKit()->GetVehicleInfo()->m_ID)
					sLog->outDebug(LOG_FILTER_NETWORKIO, "VEHICLE ID = %u.", vehicleId);

				me->setPowerType(POWER_ENERGY);
                me->SetPower(POWER_ENERGY, 0);
				me->CastSpell(me, SPELL_STONE_VISUAL);
				me->CastSpell(me, SPELL_ANIM_SIT);
				me->CastSpell(me, SPELL_ZERO_POWER);
            }

			void JustReachedHome()
            {
				me->CastSpell(me, SPELL_ANIM_SIT);
			}

			void DoAction(int32 action)
			{
				switch (action)
				{
					case ACTION_PETRIFICATION_BAR:
						events.ScheduleEvent(EVENT_PETRIFICATION_SET, 0);
						break;
				}
			}

            void EnterCombat(Unit* /*who*/)
            {
				me->RemoveAurasDueToSpell(SPELL_STONE_VISUAL, me->GetGUID());
				
				events.ScheduleEvent(EVENT_REND_FLESH, 5*IN_MILLISECONDS);
				events.ScheduleEvent(EVENT_PETRIFICATION_FIRST, 6*IN_MILLISECONDS);
				events.ScheduleEvent(EVENT_INCREASE_POWER_1, 3*IN_MILLISECONDS);
				events.ScheduleEvent(EVENT_INCREASE_POWER_2, 3475);

				if (instance)
				{
					if (Creature* cobalt = me->GetCreature(*me, instance->GetData64(DATA_COBALT_GUARDIAN)))
						cobalt->SetInCombatWithZone();
					if (Creature* jade = me->GetCreature(*me, instance->GetData64(DATA_JADE_GUARDIAN)))
						jade->SetInCombatWithZone();
					if (Creature* jasper = me->GetCreature(*me, instance->GetData64(DATA_JASPER_GUARDIAN)))
						jasper->SetInCombatWithZone();

					if (!me->isInCombat())
						me->SetInCombatWithZone();
				}
            }

			void EnterEvadeMode()
			{
				ScriptedAI::EnterEvadeMode();

				if (map)
				{
					Map::PlayerList const &PlayerList = map->GetPlayers();

					if (!PlayerList.isEmpty())
						for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
							if (Player* player = i->getSource())
							{
								player->RemoveAurasDueToSpell(SPELL_AMETHYST_PETRIFICATION_BAR);
								player->RemoveAurasDueToSpell(SPELL_COBALT_PETRIFICATION_BAR);
								player->RemoveAurasDueToSpell(SPELL_JADE_PETRIFICATION_BAR);
								player->RemoveAurasDueToSpell(SPELL_JASPER_PETRIFICATION_BAR);
								player->RemoveAurasDueToSpell(SPELL_TOTALY_PETRIFIED);
								player->SetPower(POWER_ALTERNATE_POWER, 0);
							}
				}
			}

			void DamageTaken(Unit* who, uint32& damage)
			{
				if (instance)
				{
					if (Creature* cobalt = me->GetCreature(*me, instance->GetData64(DATA_COBALT_GUARDIAN)))
						if (Creature* jade = me->GetCreature(*me, instance->GetData64(DATA_JADE_GUARDIAN)))
							if (Creature* jasper = me->GetCreature(*me, instance->GetData64(DATA_JASPER_GUARDIAN)))
							{
								if (cobalt->isAlive())
									cobalt->SetHealth(cobalt->GetHealth() - damage);

								if (jade->isAlive())
									jade->SetHealth(jade->GetHealth() - damage);

								if (jasper->isAlive())
									jasper->SetHealth(jasper->GetHealth() - damage);
							}
				}
			}

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
					return;

                events.Update(diff);
                
				if (instance)
				{
					if (!solidStone)
						if (!me->FindNearestCreature(BOSS_COBALT_GUARDIAN, 10.0f, true) &&
							!me->FindNearestCreature(BOSS_JADE_GUARDIAN, 10.0f, true) &&
							!me->FindNearestCreature(BOSS_JASPER_GUARDIAN, 10.0f, true))
						{
							me->CastSpell(me, SPELL_SOLID_STONE);
							solidStone = true;
						}
				
					if (solidStone)
						if (me->FindNearestCreature(BOSS_COBALT_GUARDIAN, 10.0f, true) ||
							me->FindNearestCreature(BOSS_JADE_GUARDIAN, 10.0f, true) ||
							me->FindNearestCreature(BOSS_JASPER_GUARDIAN, 10.0f, true))
						{
							if (me->HasAura(SPELL_SOLID_STONE))
								me->RemoveAurasDueToSpell(SPELL_SOLID_STONE, me->GetGUID());

							solidStone = false;
						}

					if (map)
					{
						Map::PlayerList const &PlayerList = map->GetPlayers();

						if (!PlayerList.isEmpty())
							for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
								if (Player* player = i->getSource())
									if (player->GetPower(POWER_ALTERNATE_POWER) == 100 && !player->HasAura(SPELL_TOTALY_PETRIFIED))
										player->CastSpell(player, SPELL_TOTALY_PETRIFIED);
					}

					while (uint32 eventId = events.ExecuteEvent())
					{
						switch (eventId)
						{
							case EVENT_REND_FLESH:
								me->CastSpell(me->getVictim(), SPELL_REND_FLESH);

								events.ScheduleEvent(EVENT_REND_FLESH, 6*IN_MILLISECONDS);
								break;

							case EVENT_PETRIFICATION_FIRST:
							{
								if (Creature* tracker = me->FindNearestCreature(NPC_THE_STONE_GUARD_TRACKER, 99999.0f, true))
									tracker->AI()->DoAction(ACTION_CHOOSE_PETRIFICATION);

								events.CancelEvent(EVENT_PETRIFICATION_FIRST);
								break;
							}

							case EVENT_PETRIFICATION_SET:
							{
								if (map)
								{
									Map::PlayerList const &PlayerList = map->GetPlayers();

									if (!PlayerList.isEmpty())
										for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
											if (Player* player = i->getSource())
											{
												player->CastSpell(player, SPELL_AMETHYST_PETRIFICATION_BAR);
												player->SetMaxPower(POWER_ALTERNATE_POWER, 100);
												player->SetPower(POWER_ALTERNATE_POWER, 5);
											}
								}

								events.ScheduleEvent(EVENT_PETRIFICATION_INCREASE_1, 6*IN_MILLISECONDS);
								events.CancelEvent(EVENT_PETRIFICATION_SET);
								break;
							}

							case EVENT_PETRIFICATION_INCREASE_1:
							{
								if (map)
								{
									Map::PlayerList const &PlayerList = map->GetPlayers();

									if (!PlayerList.isEmpty())
										for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
											if (Player* player = i->getSource())
												player->SetPower(POWER_ALTERNATE_POWER, int32(player->GetPower(POWER_ALTERNATE_POWER) + 5));
								}
								
								events.ScheduleEvent(EVENT_PETRIFICATION_INCREASE_2, 2*IN_MILLISECONDS);
								events.CancelEvent(EVENT_PETRIFICATION_INCREASE_1);
								break;
							}

							case EVENT_PETRIFICATION_INCREASE_2:
							{
								if (map)
								{
									Map::PlayerList const &PlayerList = map->GetPlayers();

									if (!PlayerList.isEmpty())
										for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
											if (Player* player = i->getSource())
												player->SetPower(POWER_ALTERNATE_POWER, int32(player->GetPower(POWER_ALTERNATE_POWER) + 5));
								}
								
								events.ScheduleEvent(EVENT_PETRIFICATION_INCREASE_3, 2*IN_MILLISECONDS);
								events.CancelEvent(EVENT_PETRIFICATION_INCREASE_2);
								break;
							}

							case EVENT_PETRIFICATION_INCREASE_3:
							{
								if (map)
								{
									Map::PlayerList const &PlayerList = map->GetPlayers();

									if (!PlayerList.isEmpty())
										for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
											if (Player* player = i->getSource())
												player->SetPower(POWER_ALTERNATE_POWER, int32(player->GetPower(POWER_ALTERNATE_POWER) + 5));
								}
								
								events.ScheduleEvent(EVENT_PETRIFICATION_INCREASE_1, 6*IN_MILLISECONDS);
								events.CancelEvent(EVENT_PETRIFICATION_INCREASE_3);
								break;
							}
							
							case EVENT_INCREASE_POWER_1:
								me->SetPower(POWER_ENERGY, int32(me->GetPower(POWER_ENERGY) + 2));

								events.ScheduleEvent(EVENT_INCREASE_POWER_1, 1150);
								break;

							case EVENT_INCREASE_POWER_2:
								me->SetPower(POWER_ENERGY, int32(me->GetPower(POWER_ENERGY) + 3));

								events.ScheduleEvent(EVENT_INCREASE_POWER_2, 1625);
								break;

							default:
								break;
						}
					}
				}

                DoMeleeAttackIfReady();
            }
        };
};

class boss_cobalt_guardian : public CreatureScript
{
    public:
        boss_cobalt_guardian() : CreatureScript("boss_cobalt_guardian") { }

		CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_cobalt_guardianAI(creature);
        }

        struct boss_cobalt_guardianAI : public ScriptedAI
        {
            boss_cobalt_guardianAI(Creature* creature) : ScriptedAI(creature)
            {
				instance = creature->GetInstanceScript();
            }

			InstanceScript* instance;
			EventMap events;
			Map* map;
			bool solidStone;

            void Reset()
            {
				events.Reset();
				solidStone = false;
				map = me->GetMap();
				
				me->setPowerType(POWER_ENERGY);
                me->SetPower(POWER_ENERGY, 0);
				me->CastSpell(me, SPELL_STONE_VISUAL);
				me->CastSpell(me, SPELL_ANIM_SIT);
				me->CastSpell(me, SPELL_ZERO_POWER);
            }
			
			void JustReachedHome()
            {
				me->CastSpell(me, SPELL_ANIM_SIT);
			}

			void DoAction(int32 action)
			{
				switch (action)
				{
					case ACTION_PETRIFICATION_BAR:
						events.ScheduleEvent(EVENT_PETRIFICATION_SET, 0);
						break;
				}
			}

            void EnterCombat(Unit* /*who*/)
            {
				me->RemoveAurasDueToSpell(SPELL_STONE_VISUAL, me->GetGUID());
				
				events.ScheduleEvent(EVENT_REND_FLESH, 5*IN_MILLISECONDS);
				events.ScheduleEvent(EVENT_PETRIFICATION_FIRST, 6*IN_MILLISECONDS);
				events.ScheduleEvent(EVENT_INCREASE_POWER_1, 3*IN_MILLISECONDS);
				events.ScheduleEvent(EVENT_INCREASE_POWER_2, 3475);

				if (instance)
				{
					if (Creature* amethyst = me->GetCreature(*me, instance->GetData64(DATA_AMETHYST_GUARDIAN)))
						amethyst->SetInCombatWithZone();
					if (Creature* jade = me->GetCreature(*me, instance->GetData64(DATA_JADE_GUARDIAN)))
						jade->SetInCombatWithZone();
					if (Creature* jasper = me->GetCreature(*me, instance->GetData64(DATA_JASPER_GUARDIAN)))
						jasper->SetInCombatWithZone();

					if (!me->isInCombat())
						me->SetInCombatWithZone();
				}
            }

			void EnterEvadeMode()
			{
				ScriptedAI::EnterEvadeMode();

				if (map)
				{
					Map::PlayerList const &PlayerList = map->GetPlayers();

					if (!PlayerList.isEmpty())
						for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
							if (Player* player = i->getSource())
							{
								player->RemoveAurasDueToSpell(SPELL_AMETHYST_PETRIFICATION_BAR);
								player->RemoveAurasDueToSpell(SPELL_COBALT_PETRIFICATION_BAR);
								player->RemoveAurasDueToSpell(SPELL_JADE_PETRIFICATION_BAR);
								player->RemoveAurasDueToSpell(SPELL_JASPER_PETRIFICATION_BAR);
								player->RemoveAurasDueToSpell(SPELL_TOTALY_PETRIFIED);
								player->SetPower(POWER_ALTERNATE_POWER, 0);
							}
				}
			}

			void DamageTaken(Unit* who, uint32& damage)
			{
				if (instance)
				{
					if (Creature* amethyst = me->GetCreature(*me, instance->GetData64(DATA_AMETHYST_GUARDIAN)))
						if (Creature* jade = me->GetCreature(*me, instance->GetData64(DATA_JADE_GUARDIAN)))
							if (Creature* jasper = me->GetCreature(*me, instance->GetData64(DATA_JASPER_GUARDIAN)))
							{
								if (amethyst->isAlive())
									amethyst->SetHealth(amethyst->GetHealth() - damage);

								if (jade->isAlive())
									jade->SetHealth(jade->GetHealth() - damage);

								if (jasper->isAlive())
									jasper->SetHealth(jasper->GetHealth() - damage);
							}
				}
			}

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
					return;

                events.Update(diff);
                
				if (instance)
				{
					if (!solidStone)
						if (!me->FindNearestCreature(BOSS_AMETHYST_GUARDIAN, 10.0f, true) &&
							!me->FindNearestCreature(BOSS_JADE_GUARDIAN, 10.0f, true) &&
							!me->FindNearestCreature(BOSS_JASPER_GUARDIAN, 10.0f, true))
						{
							me->CastSpell(me, SPELL_SOLID_STONE);
							solidStone = true;
						}
				
					if (solidStone)
						if (me->FindNearestCreature(BOSS_AMETHYST_GUARDIAN, 10.0f, true) ||
							me->FindNearestCreature(BOSS_JADE_GUARDIAN, 10.0f, true) ||
							me->FindNearestCreature(BOSS_JASPER_GUARDIAN, 10.0f, true))
						{
							if (me->HasAura(SPELL_SOLID_STONE))
								me->RemoveAurasDueToSpell(SPELL_SOLID_STONE, me->GetGUID());

							solidStone = false;
						}

					if (map)
					{
						Map::PlayerList const &PlayerList = map->GetPlayers();

						if (!PlayerList.isEmpty())
							for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
								if (Player* player = i->getSource())
									if (player->GetPower(POWER_ALTERNATE_POWER) == 100 && !player->HasAura(SPELL_TOTALY_PETRIFIED))
										player->CastSpell(player, SPELL_TOTALY_PETRIFIED);
					}

					while (uint32 eventId = events.ExecuteEvent())
					{
						switch (eventId)
						{
							case EVENT_REND_FLESH:
								me->CastSpell(me->getVictim(), SPELL_REND_FLESH);

								events.ScheduleEvent(EVENT_REND_FLESH, 6*IN_MILLISECONDS);
								break;

							case EVENT_PETRIFICATION_FIRST:
							{
								if (Creature* tracker = me->FindNearestCreature(NPC_THE_STONE_GUARD_TRACKER, 99999.0f, true))
									tracker->AI()->DoAction(ACTION_CHOOSE_PETRIFICATION);

								events.CancelEvent(EVENT_PETRIFICATION_FIRST);
								break;
							}

							case EVENT_PETRIFICATION_SET:
							{
								if (map)
								{
									Map::PlayerList const &PlayerList = map->GetPlayers();

									if (!PlayerList.isEmpty())
										for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
											if (Player* player = i->getSource())
											{
												player->CastSpell(player, SPELL_COBALT_PETRIFICATION_BAR);
												player->SetMaxPower(POWER_ALTERNATE_POWER, 100);
												player->SetPower(POWER_ALTERNATE_POWER, 5);
											}
								}

								events.ScheduleEvent(EVENT_PETRIFICATION_INCREASE_1, 6*IN_MILLISECONDS);
								events.CancelEvent(EVENT_PETRIFICATION_SET);
								break;
							}

							case EVENT_PETRIFICATION_INCREASE_1:
							{
								if (map)
								{
									Map::PlayerList const &PlayerList = map->GetPlayers();

									if (!PlayerList.isEmpty())
										for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
											if (Player* player = i->getSource())
												player->SetPower(POWER_ALTERNATE_POWER, int32(player->GetPower(POWER_ALTERNATE_POWER) + 5));
								}
								
								events.ScheduleEvent(EVENT_PETRIFICATION_INCREASE_2, 2*IN_MILLISECONDS);
								events.CancelEvent(EVENT_PETRIFICATION_INCREASE_1);
								break;
							}

							case EVENT_PETRIFICATION_INCREASE_2:
							{
								if (map)
								{
									Map::PlayerList const &PlayerList = map->GetPlayers();

									if (!PlayerList.isEmpty())
										for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
											if (Player* player = i->getSource())
												player->SetPower(POWER_ALTERNATE_POWER, int32(player->GetPower(POWER_ALTERNATE_POWER) + 5));
								}
								
								events.ScheduleEvent(EVENT_PETRIFICATION_INCREASE_3, 2*IN_MILLISECONDS);
								events.CancelEvent(EVENT_PETRIFICATION_INCREASE_2);
								break;
							}

							case EVENT_PETRIFICATION_INCREASE_3:
							{
								if (map)
								{
									Map::PlayerList const &PlayerList = map->GetPlayers();

									if (!PlayerList.isEmpty())
										for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
											if (Player* player = i->getSource())
												player->SetPower(POWER_ALTERNATE_POWER, int32(player->GetPower(POWER_ALTERNATE_POWER) + 5));
								}
								
								events.ScheduleEvent(EVENT_PETRIFICATION_INCREASE_1, 6*IN_MILLISECONDS);
								events.CancelEvent(EVENT_PETRIFICATION_INCREASE_3);
								break;
							}
							
							case EVENT_INCREASE_POWER_1:
								me->SetPower(POWER_ENERGY, int32(me->GetPower(POWER_ENERGY) + 2));

								events.ScheduleEvent(EVENT_INCREASE_POWER_1, 1150);
								break;

							case EVENT_INCREASE_POWER_2:
								me->SetPower(POWER_ENERGY, int32(me->GetPower(POWER_ENERGY) + 3));

								events.ScheduleEvent(EVENT_INCREASE_POWER_2, 1625);
								break;

							default:
								break;
						}
					}
				}

                DoMeleeAttackIfReady();
            }
        };
};

class boss_jade_guardian : public CreatureScript
{
    public:
        boss_jade_guardian() : CreatureScript("boss_jade_guardian") { }

		CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_jade_guardianAI(creature);
        }

        struct boss_jade_guardianAI : public ScriptedAI
        {
            boss_jade_guardianAI(Creature* creature) : ScriptedAI(creature)
            {
				instance = creature->GetInstanceScript();
            }

			InstanceScript* instance;
			EventMap events;
			Map* map;
			bool solidStone;

            void Reset()
            {
				events.Reset();
				solidStone = false;
				map = me->GetMap();

				me->setPowerType(POWER_ENERGY);
                me->SetPower(POWER_ENERGY, 0);
				me->CastSpell(me, SPELL_STONE_VISUAL);
				me->CastSpell(me, SPELL_ANIM_SIT);
				me->CastSpell(me, SPELL_ZERO_POWER);
            }
			
			void JustReachedHome()
            {
				me->CastSpell(me, SPELL_ANIM_SIT);
			}

			void DoAction(int32 action)
			{
				switch (action)
				{
					case ACTION_PETRIFICATION_BAR:
						events.ScheduleEvent(EVENT_PETRIFICATION_SET, 0);
						break;
				}
			}

            void EnterCombat(Unit* /*who*/)
            {
				me->RemoveAurasDueToSpell(SPELL_STONE_VISUAL, me->GetGUID());
				
				events.ScheduleEvent(EVENT_REND_FLESH, 5*IN_MILLISECONDS);
				events.ScheduleEvent(EVENT_PETRIFICATION_FIRST, 6*IN_MILLISECONDS);
				events.ScheduleEvent(EVENT_INCREASE_POWER_1, 3*IN_MILLISECONDS);
				events.ScheduleEvent(EVENT_INCREASE_POWER_2, 3475);

				if (instance)
				{
					if (Creature* amethyst = me->GetCreature(*me, instance->GetData64(DATA_AMETHYST_GUARDIAN)))
						amethyst->SetInCombatWithZone();
					if (Creature* cobalt = me->GetCreature(*me, instance->GetData64(DATA_COBALT_GUARDIAN)))
						cobalt->SetInCombatWithZone();
					if (Creature* jasper = me->GetCreature(*me, instance->GetData64(DATA_JASPER_GUARDIAN)))
						jasper->SetInCombatWithZone();

					if (!me->isInCombat())
						me->SetInCombatWithZone();
				}
            }

			void EnterEvadeMode()
			{
				ScriptedAI::EnterEvadeMode();

				if (map)
				{
					Map::PlayerList const &PlayerList = map->GetPlayers();

					if (!PlayerList.isEmpty())
						for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
							if (Player* player = i->getSource())
							{
								player->RemoveAurasDueToSpell(SPELL_AMETHYST_PETRIFICATION_BAR);
								player->RemoveAurasDueToSpell(SPELL_COBALT_PETRIFICATION_BAR);
								player->RemoveAurasDueToSpell(SPELL_JADE_PETRIFICATION_BAR);
								player->RemoveAurasDueToSpell(SPELL_JASPER_PETRIFICATION_BAR);
								player->RemoveAurasDueToSpell(SPELL_TOTALY_PETRIFIED);
								player->SetPower(POWER_ALTERNATE_POWER, 0);
							}
				}
			}

			void DamageTaken(Unit* who, uint32& damage)
			{
				if (instance)
				{
					if (Creature* amethyst = me->GetCreature(*me, instance->GetData64(DATA_AMETHYST_GUARDIAN)))
						if (Creature* cobalt = me->GetCreature(*me, instance->GetData64(DATA_COBALT_GUARDIAN)))
							if (Creature* jasper = me->GetCreature(*me, instance->GetData64(DATA_JASPER_GUARDIAN)))
							{
								if (amethyst->isAlive())
									amethyst->SetHealth(amethyst->GetHealth() - damage);

								if (cobalt->isAlive())
									cobalt->SetHealth(cobalt->GetHealth() - damage);

								if (jasper->isAlive())
									jasper->SetHealth(jasper->GetHealth() - damage);
							}
				}
			}

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
					return;

                events.Update(diff);
                
				if (instance)
				{
					if (!solidStone)
						if (!me->FindNearestCreature(BOSS_AMETHYST_GUARDIAN, 10.0f, true) &&
							!me->FindNearestCreature(BOSS_COBALT_GUARDIAN, 10.0f, true) &&
							!me->FindNearestCreature(BOSS_JASPER_GUARDIAN, 10.0f, true))
						{
							me->CastSpell(me, SPELL_SOLID_STONE);
							solidStone = true;
						}
				
					if (solidStone)
						if (me->FindNearestCreature(BOSS_AMETHYST_GUARDIAN, 10.0f, true) ||
							me->FindNearestCreature(BOSS_COBALT_GUARDIAN, 10.0f, true) ||
							me->FindNearestCreature(BOSS_JASPER_GUARDIAN, 10.0f, true))
						{
							if (me->HasAura(SPELL_SOLID_STONE))
								me->RemoveAurasDueToSpell(SPELL_SOLID_STONE, me->GetGUID());

							solidStone = false;
						}

					if (map)
					{
						Map::PlayerList const &PlayerList = map->GetPlayers();

						if (!PlayerList.isEmpty())
							for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
								if (Player* player = i->getSource())
									if (player->GetPower(POWER_ALTERNATE_POWER) == 100 && !player->HasAura(SPELL_TOTALY_PETRIFIED))
										player->CastSpell(player, SPELL_TOTALY_PETRIFIED);
					}

					while (uint32 eventId = events.ExecuteEvent())
					{
						switch (eventId)
						{
							case EVENT_REND_FLESH:
								me->CastSpell(me->getVictim(), SPELL_REND_FLESH);

								events.ScheduleEvent(EVENT_REND_FLESH, 6*IN_MILLISECONDS);
								break;

							case EVENT_PETRIFICATION_FIRST:
							{
								if (Creature* tracker = me->FindNearestCreature(NPC_THE_STONE_GUARD_TRACKER, 99999.0f, true))
									tracker->AI()->DoAction(ACTION_CHOOSE_PETRIFICATION);

								events.CancelEvent(EVENT_PETRIFICATION_FIRST);
								break;
							}

							case EVENT_PETRIFICATION_SET:
							{
								if (map)
								{
									Map::PlayerList const &PlayerList = map->GetPlayers();

									if (!PlayerList.isEmpty())
										for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
											if (Player* player = i->getSource())
											{
												player->CastSpell(player, SPELL_JADE_PETRIFICATION_BAR);
												player->SetMaxPower(POWER_ALTERNATE_POWER, 100);
												player->SetPower(POWER_ALTERNATE_POWER, 5);
											}
								}

								events.ScheduleEvent(EVENT_PETRIFICATION_INCREASE_1, 6*IN_MILLISECONDS);
								events.CancelEvent(EVENT_PETRIFICATION_SET);
								break;
							}

							case EVENT_PETRIFICATION_INCREASE_1:
							{
								if (map)
								{
									Map::PlayerList const &PlayerList = map->GetPlayers();

									if (!PlayerList.isEmpty())
										for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
											if (Player* player = i->getSource())
												player->SetPower(POWER_ALTERNATE_POWER, int32(player->GetPower(POWER_ALTERNATE_POWER) + 5));
								}
								
								events.ScheduleEvent(EVENT_PETRIFICATION_INCREASE_2, 2*IN_MILLISECONDS);
								events.CancelEvent(EVENT_PETRIFICATION_INCREASE_1);
								break;
							}

							case EVENT_PETRIFICATION_INCREASE_2:
							{
								if (map)
								{
									Map::PlayerList const &PlayerList = map->GetPlayers();

									if (!PlayerList.isEmpty())
										for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
											if (Player* player = i->getSource())
												player->SetPower(POWER_ALTERNATE_POWER, int32(player->GetPower(POWER_ALTERNATE_POWER) + 5));
								}
								
								events.ScheduleEvent(EVENT_PETRIFICATION_INCREASE_3, 2*IN_MILLISECONDS);
								events.CancelEvent(EVENT_PETRIFICATION_INCREASE_2);
								break;
							}

							case EVENT_PETRIFICATION_INCREASE_3:
							{
								if (map)
								{
									Map::PlayerList const &PlayerList = map->GetPlayers();

									if (!PlayerList.isEmpty())
										for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
											if (Player* player = i->getSource())
												player->SetPower(POWER_ALTERNATE_POWER, int32(player->GetPower(POWER_ALTERNATE_POWER) + 5));
								}
								
								events.ScheduleEvent(EVENT_PETRIFICATION_INCREASE_1, 6*IN_MILLISECONDS);
								events.CancelEvent(EVENT_PETRIFICATION_INCREASE_3);
								break;
							}
							
							case EVENT_INCREASE_POWER_1:
								me->SetPower(POWER_ENERGY, int32(me->GetPower(POWER_ENERGY) + 2));

								events.ScheduleEvent(EVENT_INCREASE_POWER_1, 1150);
								break;

							case EVENT_INCREASE_POWER_2:
								me->SetPower(POWER_ENERGY, int32(me->GetPower(POWER_ENERGY) + 3));

								events.ScheduleEvent(EVENT_INCREASE_POWER_2, 1625);
								break;

							default:
								break;
						}
					}
				}

                DoMeleeAttackIfReady();
            }
        };
};

class boss_jasper_guardian : public CreatureScript
{
    public:
        boss_jasper_guardian() : CreatureScript("boss_jasper_guardian") { }

		CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_jasper_guardianAI(creature);
        }

        struct boss_jasper_guardianAI : public ScriptedAI
        {
            boss_jasper_guardianAI(Creature* creature) : ScriptedAI(creature)
            {
				instance = creature->GetInstanceScript();
            }

			InstanceScript* instance;
			EventMap events;
			Map* map;
			bool solidStone;

            void Reset()
            {
				events.Reset();
				solidStone = false;
				map = me->GetMap();

				me->setPowerType(POWER_ENERGY);
                me->SetPower(POWER_ENERGY, 0);
				me->CastSpell(me, SPELL_STONE_VISUAL);
				me->CastSpell(me, SPELL_ANIM_SIT);
				me->CastSpell(me, SPELL_ZERO_POWER);
            }
			
			void JustReachedHome()
            {
				me->CastSpell(me, SPELL_ANIM_SIT);
			}

			void DoAction(int32 action)
			{
				switch (action)
				{
					case ACTION_PETRIFICATION_BAR:
						events.ScheduleEvent(EVENT_PETRIFICATION_SET, 0);
						break;
				}
			}

            void EnterCombat(Unit* /*who*/)
            {
				me->RemoveAurasDueToSpell(SPELL_STONE_VISUAL, me->GetGUID());
				
				events.ScheduleEvent(EVENT_REND_FLESH, 5*IN_MILLISECONDS);
				events.ScheduleEvent(EVENT_PETRIFICATION_FIRST, 6*IN_MILLISECONDS);
				events.ScheduleEvent(EVENT_INCREASE_POWER_1, 3*IN_MILLISECONDS);
				events.ScheduleEvent(EVENT_INCREASE_POWER_2, 3475);

				if (instance)
				{
					if (Creature* amethyst = me->GetCreature(*me, instance->GetData64(DATA_AMETHYST_GUARDIAN)))
						amethyst->SetInCombatWithZone();
					if (Creature* cobalt = me->GetCreature(*me, instance->GetData64(DATA_COBALT_GUARDIAN)))
						cobalt->SetInCombatWithZone();
					if (Creature* jade = me->GetCreature(*me, instance->GetData64(DATA_JADE_GUARDIAN)))
						jade->SetInCombatWithZone();

					if (!me->isInCombat())
						me->SetInCombatWithZone();
				}
            }

			void EnterEvadeMode()
			{
				ScriptedAI::EnterEvadeMode();

				if (map)
				{
					Map::PlayerList const &PlayerList = map->GetPlayers();

					if (!PlayerList.isEmpty())
						for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
							if (Player* player = i->getSource())
							{
								player->RemoveAurasDueToSpell(SPELL_AMETHYST_PETRIFICATION_BAR);
								player->RemoveAurasDueToSpell(SPELL_COBALT_PETRIFICATION_BAR);
								player->RemoveAurasDueToSpell(SPELL_JADE_PETRIFICATION_BAR);
								player->RemoveAurasDueToSpell(SPELL_JASPER_PETRIFICATION_BAR);
								player->RemoveAurasDueToSpell(SPELL_TOTALY_PETRIFIED);
								player->SetPower(POWER_ALTERNATE_POWER, 0);
							}
				}
			}

			void DamageTaken(Unit* who, uint32& damage)
			{
				if (instance)
				{
					if (Creature* amethyst = me->GetCreature(*me, instance->GetData64(DATA_AMETHYST_GUARDIAN)))
						if (Creature* cobalt = me->GetCreature(*me, instance->GetData64(DATA_COBALT_GUARDIAN)))
							if (Creature* jade = me->GetCreature(*me, instance->GetData64(DATA_JADE_GUARDIAN)))
							{
								if (amethyst->isAlive())
									amethyst->SetHealth(amethyst->GetHealth() - damage);

								if (cobalt->isAlive())
									cobalt->SetHealth(cobalt->GetHealth() - damage);

								if (jade->isAlive())
									jade->SetHealth(jade->GetHealth() - damage);
							}
				}
			}

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
					return;

                events.Update(diff);
                
				if (instance)
				{
					if (!solidStone)
						if (!me->FindNearestCreature(BOSS_AMETHYST_GUARDIAN, 10.0f, true) &&
							!me->FindNearestCreature(BOSS_COBALT_GUARDIAN, 10.0f, true) &&
							!me->FindNearestCreature(BOSS_JADE_GUARDIAN, 10.0f, true))
						{
							me->CastSpell(me, SPELL_SOLID_STONE);
							solidStone = true;
						}
				
					if (solidStone)
						if (me->FindNearestCreature(BOSS_AMETHYST_GUARDIAN, 10.0f, true) ||
							me->FindNearestCreature(BOSS_COBALT_GUARDIAN, 10.0f, true) ||
							me->FindNearestCreature(BOSS_JADE_GUARDIAN, 10.0f, true))
						{
							if (me->HasAura(SPELL_SOLID_STONE))
								me->RemoveAurasDueToSpell(SPELL_SOLID_STONE, me->GetGUID());

							solidStone = false;
						}

					if (map)
					{
						Map::PlayerList const &PlayerList = map->GetPlayers();

						if (!PlayerList.isEmpty())
							for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
								if (Player* player = i->getSource())
									if (player->GetPower(POWER_ALTERNATE_POWER) == 100 && !player->HasAura(SPELL_TOTALY_PETRIFIED))
										player->CastSpell(player, SPELL_TOTALY_PETRIFIED);
					}

					while (uint32 eventId = events.ExecuteEvent())
					{
						switch (eventId)
						{
							case EVENT_REND_FLESH:
								me->CastSpell(me->getVictim(), SPELL_REND_FLESH);

								events.ScheduleEvent(EVENT_REND_FLESH, 6*IN_MILLISECONDS);
								break;

							case EVENT_PETRIFICATION_FIRST:
							{
								if (Creature* tracker = me->FindNearestCreature(NPC_THE_STONE_GUARD_TRACKER, 99999.0f, true))
									tracker->AI()->DoAction(ACTION_CHOOSE_PETRIFICATION);

								events.CancelEvent(EVENT_PETRIFICATION_FIRST);
								break;
							}

							case EVENT_PETRIFICATION_SET:
							{
								if (map)
								{
									Map::PlayerList const &PlayerList = map->GetPlayers();

									if (!PlayerList.isEmpty())
										for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
											if (Player* player = i->getSource())
											{
												player->CastSpell(player, SPELL_JASPER_PETRIFICATION_BAR);
												player->SetMaxPower(POWER_ALTERNATE_POWER, 100);
												player->SetPower(POWER_ALTERNATE_POWER, 5);
											}
								}

								events.ScheduleEvent(EVENT_PETRIFICATION_INCREASE_1, 6*IN_MILLISECONDS);
								events.CancelEvent(EVENT_PETRIFICATION_SET);
								break;
							}

							case EVENT_PETRIFICATION_INCREASE_1:
							{
								if (map)
								{
									Map::PlayerList const &PlayerList = map->GetPlayers();

									if (!PlayerList.isEmpty())
										for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
											if (Player* player = i->getSource())
												player->SetPower(POWER_ALTERNATE_POWER, int32(player->GetPower(POWER_ALTERNATE_POWER) + 5));
								}
								
								events.ScheduleEvent(EVENT_PETRIFICATION_INCREASE_2, 2*IN_MILLISECONDS);
								events.CancelEvent(EVENT_PETRIFICATION_INCREASE_1);
								break;
							}

							case EVENT_PETRIFICATION_INCREASE_2:
							{
								if (map)
								{
									Map::PlayerList const &PlayerList = map->GetPlayers();

									if (!PlayerList.isEmpty())
										for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
											if (Player* player = i->getSource())
												player->SetPower(POWER_ALTERNATE_POWER, int32(player->GetPower(POWER_ALTERNATE_POWER) + 5));
								}
								
								events.ScheduleEvent(EVENT_PETRIFICATION_INCREASE_3, 2*IN_MILLISECONDS);
								events.CancelEvent(EVENT_PETRIFICATION_INCREASE_2);
								break;
							}

							case EVENT_PETRIFICATION_INCREASE_3:
							{
								if (map)
								{
									Map::PlayerList const &PlayerList = map->GetPlayers();

									if (!PlayerList.isEmpty())
										for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
											if (Player* player = i->getSource())
												player->SetPower(POWER_ALTERNATE_POWER, 5);
								}
								
								events.ScheduleEvent(EVENT_PETRIFICATION_INCREASE_1, 6*IN_MILLISECONDS);
								events.CancelEvent(EVENT_PETRIFICATION_INCREASE_3);
								break;
							}
							
							case EVENT_INCREASE_POWER_1:
								me->SetPower(POWER_ENERGY, int32(me->GetPower(POWER_ENERGY) + 2));

								events.ScheduleEvent(EVENT_INCREASE_POWER_1, 1150);
								break;

							case EVENT_INCREASE_POWER_2:
								me->SetPower(POWER_ENERGY, int32(me->GetPower(POWER_ENERGY) + 3));

								events.ScheduleEvent(EVENT_INCREASE_POWER_2, 1625);
								break;

							default:
								break;
						}
					}
				}

                DoMeleeAttackIfReady();
            }
        };
};

class npc_the_stone_guard_tracker : public CreatureScript
{
public:
	npc_the_stone_guard_tracker() : CreatureScript("npc_the_stone_guard_tracker") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_the_stone_guard_trackerAI(creature);
	}

	struct npc_the_stone_guard_trackerAI : public ScriptedAI
	{
		npc_the_stone_guard_trackerAI(Creature *creature) : ScriptedAI(creature)
		{
			instance = creature->GetInstanceScript();
		}

		InstanceScript* instance;
		EventMap events;
		uint64 lastGuardianPetrificationGUID;

		void Reset()
        {
			events.Reset();
			lastGuardianPetrificationGUID = 0;
        }

		void DoAction(int32 action)
        {
            switch (action)
            {
				case ACTION_CHOOSE_PETRIFICATION:
					events.ScheduleEvent(EVENT_CHOOSE_PETRIFICATION, 6*IN_MILLISECONDS);
					break;
			}
		}

		bool IsAmethystEligible()
		{
			if (instance)
			{
				if (Creature* amethyst = me->GetCreature(*me, instance->GetData64(DATA_AMETHYST_GUARDIAN)))
				{
					if (!amethyst->isAlive() || amethyst->GetGUID() == lastGuardianPetrificationGUID)
						return false;
				}
				else return false;
			}

			return true;
		}

		bool IsCobaltEligible()
		{
			if (instance)
			{
				if (Creature* cobalt = me->GetCreature(*me, instance->GetData64(DATA_COBALT_GUARDIAN)))
				{
					if (!cobalt->isAlive() || cobalt->GetGUID() == lastGuardianPetrificationGUID)
						return false;
				}
				else return false;
			}

			return true;
		}

		bool IsJadeEligible()
		{
			if (instance)
			{
				if (Creature* jade = me->GetCreature(*me, instance->GetData64(DATA_JADE_GUARDIAN)))
				{
					if (!jade->isAlive() || jade->GetGUID() == lastGuardianPetrificationGUID)
						return false;
				}
				else return false;
			}

			return true;
		}

		bool IsJasperEligible()
		{
			if (instance)
			{
				if (Creature* jasper = me->GetCreature(*me, instance->GetData64(DATA_JASPER_GUARDIAN)))
				{
					if (!jasper->isAlive() || jasper->GetGUID() == lastGuardianPetrificationGUID)
						return false;
				}
				else return false;
			}

			return true;
		}

		void UpdateAI(uint32 diff)
		{
			events.Update(diff);

			if (instance)
			{
				while (uint32 eventId = events.ExecuteEvent())
				{
					switch (eventId)
					{
						case EVENT_CHOOSE_PETRIFICATION:
						{
							if (IsAmethystEligible() && IsCobaltEligible() && IsJadeEligible() && IsJasperEligible())
							{
								if (Creature* guardian = me->GetCreature(*me, instance->GetData64(RAND(DATA_AMETHYST_GUARDIAN,
																									   DATA_COBALT_GUARDIAN,
																									   DATA_JADE_GUARDIAN,
																									   DATA_JASPER_GUARDIAN))))
								{
									guardian->AI()->DoAction(ACTION_PETRIFICATION_BAR);
									lastGuardianPetrificationGUID = guardian->GetGUID();
								}
							}

							if (!IsAmethystEligible() && IsCobaltEligible() && IsJadeEligible() && IsJasperEligible())
							{
								if (Creature* guardian = me->GetCreature(*me, instance->GetData64(RAND(DATA_COBALT_GUARDIAN,
																									   DATA_JADE_GUARDIAN,
																									   DATA_JASPER_GUARDIAN))))
								{
									guardian->AI()->DoAction(ACTION_PETRIFICATION_BAR);
									lastGuardianPetrificationGUID = guardian->GetGUID();
								}
							}

							if (IsAmethystEligible() && !IsCobaltEligible() && IsJadeEligible() && IsJasperEligible())
							{
								if (Creature* guardian = me->GetCreature(*me, instance->GetData64(RAND(DATA_AMETHYST_GUARDIAN,
																									   DATA_JADE_GUARDIAN,
																									   DATA_JASPER_GUARDIAN))))
								{
									guardian->AI()->DoAction(ACTION_PETRIFICATION_BAR);
									lastGuardianPetrificationGUID = guardian->GetGUID();
								}
							}

							if (IsAmethystEligible() && IsCobaltEligible() && !IsJadeEligible() && IsJasperEligible())
							{
								if (Creature* guardian = me->GetCreature(*me, instance->GetData64(RAND(DATA_AMETHYST_GUARDIAN,
																									   DATA_COBALT_GUARDIAN,
																									   DATA_JASPER_GUARDIAN))))
								{
									guardian->AI()->DoAction(ACTION_PETRIFICATION_BAR);
									lastGuardianPetrificationGUID = guardian->GetGUID();
								}
							}

							if (IsAmethystEligible() && IsCobaltEligible() && IsJadeEligible() && !IsJasperEligible())
							{
								if (Creature* guardian = me->GetCreature(*me, instance->GetData64(RAND(DATA_AMETHYST_GUARDIAN,
																									   DATA_COBALT_GUARDIAN,
																									   DATA_JADE_GUARDIAN))))
								{
									guardian->AI()->DoAction(ACTION_PETRIFICATION_BAR);
									lastGuardianPetrificationGUID = guardian->GetGUID();
								}
							}

							if (!IsAmethystEligible() && !IsCobaltEligible() && IsJadeEligible() && IsJasperEligible())
							{
								if (Creature* guardian = me->GetCreature(*me, instance->GetData64(RAND(DATA_JADE_GUARDIAN,
																									   DATA_JASPER_GUARDIAN))))
								{
									guardian->AI()->DoAction(ACTION_PETRIFICATION_BAR);
									lastGuardianPetrificationGUID = guardian->GetGUID();
								}
							}


							if (!IsAmethystEligible() && IsCobaltEligible() && !IsJadeEligible() && IsJasperEligible())
							{
								if (Creature* guardian = me->GetCreature(*me, instance->GetData64(RAND(DATA_COBALT_GUARDIAN,
																									   DATA_JASPER_GUARDIAN))))
								{
									guardian->AI()->DoAction(ACTION_PETRIFICATION_BAR);
									lastGuardianPetrificationGUID = guardian->GetGUID();
								}
							}

							if (!IsAmethystEligible() && IsCobaltEligible() && IsJadeEligible() && !IsJasperEligible())
							{
								if (Creature* guardian = me->GetCreature(*me, instance->GetData64(RAND(DATA_COBALT_GUARDIAN,
																									   DATA_JADE_GUARDIAN))))
								{
									guardian->AI()->DoAction(ACTION_PETRIFICATION_BAR);
									lastGuardianPetrificationGUID = guardian->GetGUID();
								}
							}

							if (IsAmethystEligible() && !IsCobaltEligible() && !IsJadeEligible() && IsJasperEligible())
							{
								if (Creature* guardian = me->GetCreature(*me, instance->GetData64(RAND(DATA_AMETHYST_GUARDIAN,
																									   DATA_JASPER_GUARDIAN))))
								{
									guardian->AI()->DoAction(ACTION_PETRIFICATION_BAR);
									lastGuardianPetrificationGUID = guardian->GetGUID();
								}
							}

							if (IsAmethystEligible() && !IsCobaltEligible() && IsJadeEligible() && !IsJasperEligible())
							{
								if (Creature* guardian = me->GetCreature(*me, instance->GetData64(RAND(DATA_AMETHYST_GUARDIAN,
																									   DATA_JADE_GUARDIAN))))
								{
									guardian->AI()->DoAction(ACTION_PETRIFICATION_BAR);
									lastGuardianPetrificationGUID = guardian->GetGUID();
								}
							}

							if (IsAmethystEligible() && IsCobaltEligible() && !IsJadeEligible() && !IsJasperEligible())
							{
								if (Creature* guardian = me->GetCreature(*me, instance->GetData64(RAND(DATA_AMETHYST_GUARDIAN,
																									   DATA_COBALT_GUARDIAN))))
								{
									guardian->AI()->DoAction(ACTION_PETRIFICATION_BAR);
									lastGuardianPetrificationGUID = guardian->GetGUID();
								}
							}

							if (IsAmethystEligible() && IsCobaltEligible() && !IsJadeEligible() && !IsJasperEligible())
							{
								if (Creature* guardian = me->GetCreature(*me, instance->GetData64(RAND(DATA_AMETHYST_GUARDIAN,
																									   DATA_COBALT_GUARDIAN))))
								{
									guardian->AI()->DoAction(ACTION_PETRIFICATION_BAR);
									lastGuardianPetrificationGUID = guardian->GetGUID();
								}
							}

							if (IsAmethystEligible() && !IsCobaltEligible() && !IsJadeEligible() && !IsJasperEligible())
							{
								if (Creature* guardian = me->GetCreature(*me, instance->GetData64(DATA_AMETHYST_GUARDIAN)))
								{
									guardian->AI()->DoAction(ACTION_PETRIFICATION_BAR);
									lastGuardianPetrificationGUID = guardian->GetGUID();
								}
							}

							if (!IsAmethystEligible() && IsCobaltEligible() && !IsJadeEligible() && !IsJasperEligible())
							{
								if (Creature* guardian = me->GetCreature(*me, instance->GetData64(DATA_COBALT_GUARDIAN)))
								{
									guardian->AI()->DoAction(ACTION_PETRIFICATION_BAR);
									lastGuardianPetrificationGUID = guardian->GetGUID();
								}
							}

							if (!IsAmethystEligible() && !IsCobaltEligible() && IsJadeEligible() && !IsJasperEligible())
							{
								if (Creature* guardian = me->GetCreature(*me, instance->GetData64(DATA_JADE_GUARDIAN)))
								{
									guardian->AI()->DoAction(ACTION_PETRIFICATION_BAR);
									lastGuardianPetrificationGUID = guardian->GetGUID();
								}
							}

							if (!IsAmethystEligible() && !IsCobaltEligible() && !IsJadeEligible() && IsJasperEligible())
							{
								if (Creature* guardian = me->GetCreature(*me, instance->GetData64(DATA_JASPER_GUARDIAN)))
								{
									guardian->AI()->DoAction(ACTION_PETRIFICATION_BAR);
									lastGuardianPetrificationGUID = guardian->GetGUID();
								}
							}

							events.CancelEvent(EVENT_CHOOSE_PETRIFICATION);
							break;
						}

						default:
							break;
					}
				}
			}
		}
	};
};

void AddSC_boss_the_stone_guard()
{
    new boss_amethyst_guardian();
	new boss_cobalt_guardian();
	new boss_jade_guardian();
	new boss_jasper_guardian();
	new npc_the_stone_guard_tracker();
}