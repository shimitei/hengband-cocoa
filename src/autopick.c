/* File: autopick.c */

/* Purpose: Object Auto-picker/Destroyer */

/*
 * Copyright (c) 2002  Mogami
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

/*
 * Macros for Keywords
 */
#define FLG_ALL 0
#define FLG_COLLECTING 1
#define FLG_UNIDENTIFIED 2
#define FLG_IDENTIFIED 3
#define FLG_STAR_IDENTIFIED 4
#define FLG_NAMELESS 5
#define FLG_UNAWARE 6
#define FLG_WORTHLESS 7
#define FLG_BOOSTED 8
#define FLG_MORE_THAN 9
#define FLG_DICE 10
#define FLG_WANTED 11
#define FLG_UNIQUE 12
#define FLG_HUMAN 13
#define FLG_UNREADABLE 14
#define FLG_REALM1 15
#define FLG_REALM2 16
#define FLG_FIRST 17
#define FLG_SECOND 18
#define FLG_THIRD 19
#define FLG_FOURTH 20
#define FLG_ITEMS 21
#define FLG_ARTIFACTS 22
#define FLG_WEAPONS 23
#define FLG_ARMORS 24
#define FLG_MISSILES 25
#define FLG_DEVICES 26
#define FLG_LIGHTS 27
#define FLG_JUNKS 28
#define FLG_SPELLBOOKS 29
#define FLG_HAFTED 30
#define FLG_SHIELDS 31
#define FLG_BOWS 32
#define FLG_RINGS 33
#define FLG_AMULETS 34
#define FLG_SUITS 35
#define FLG_CLOAKS 36
#define FLG_HELMS 37
#define FLG_GLOVES 38
#define FLG_BOOTS 39

#ifdef JP

#define KEY_ALL "���٤Ƥ�"
#define KEY_COLLECTING "�������"
#define KEY_UNIDENTIFIED "̤�����"
#define KEY_IDENTIFIED "����Ѥߤ�"
#define KEY_STAR_IDENTIFIED "*����*�Ѥߤ�"
#define KEY_NAMELESS "̵�ä�"
#define KEY_UNAWARE "̤Ƚ����"
#define KEY_WORTHLESS "̵���ͤ�"
#define KEY_BOOSTED "�������ܤΰ㤦"
#define KEY_MORE_THAN  "��������"
#define KEY_DICE  "�ʾ��"
#define KEY_WANTED "�޶���"
#define KEY_UNIQUE "��ˡ�������󥹥�����"
#define KEY_HUMAN "�ʹ֤�"
#define KEY_UNREADABLE "�ɤ�ʤ�"
#define KEY_REALM1 "����ΰ��"
#define KEY_REALM2 "�����ΰ��"
#define KEY_FIRST "1���ܤ�"
#define KEY_SECOND "2���ܤ�"
#define KEY_THIRD "3���ܤ�"
#define KEY_FOURTH "4���ܤ�"
#define KEY_ITEMS "�����ƥ�"
#define KEY_ARTIFACTS "�����ƥ��ե�����"
#define KEY_WEAPONS "���"
#define KEY_ARMORS "�ɶ�"
#define KEY_MISSILES "��"
#define KEY_DEVICES "��ˡ�����ƥ�"
#define KEY_LIGHTS "����"
#define KEY_JUNKS "���餯��"
#define KEY_SPELLBOOKS "��ˡ��"
#define KEY_HAFTED "�ߴ�"
#define KEY_SHIELDS "��"
#define KEY_BOWS "��"
#define KEY_RINGS "����"
#define KEY_AMULETS "���ߥ��å�"
#define KEY_SUITS "��"
#define KEY_CLOAKS "��������"
#define KEY_HELMS "��"
#define KEY_GLOVES "�Ƽ�"
#define KEY_BOOTS "��"

#else 

#define KEY_ALL "all"
#define KEY_COLLECTING "collecting"
#define KEY_UNIDENTIFIED "unidentified"
#define KEY_IDENTIFIED "identified"
#define KEY_STAR_IDENTIFIED "*identified*"
#define KEY_NAMELESS "nameless"
#define KEY_UNAWARE "unaware"
#define KEY_WORTHLESS "worthless"
#define KEY_BOOSTED "dice boosted"
#define KEY_MORE_THAN  "more than "
#define KEY_DICE  " dice "
#define KEY_WANTED "wanted"
#define KEY_UNIQUE "unique monster's"
#define KEY_HUMAN "human"
#define KEY_UNREADABLE "unreadable"
#define KEY_REALM1 "first realm's"
#define KEY_REALM2 "second realm's"
#define KEY_FIRST "first"
#define KEY_SECOND "second"
#define KEY_THIRD "third"
#define KEY_FOURTH "fourth"
#define KEY_ITEMS "items"
#define KEY_ARTIFACTS "artifacts"
#define KEY_WEAPONS "weapons"
#define KEY_ARMORS "armors"
#define KEY_MISSILES "missiles"
#define KEY_DEVICES "magical devices"
#define KEY_LIGHTS "lights"
#define KEY_JUNKS "junks"
#define KEY_SPELLBOOKS "spellbooks"
#define KEY_HAFTED "hafted weapons"
#define KEY_SHIELDS "shields"
#define KEY_BOWS "bows"
#define KEY_RINGS "rings"
#define KEY_AMULETS "amulets"
#define KEY_SUITS "suits"
#define KEY_CLOAKS "cloaks"
#define KEY_HELMS "helms"
#define KEY_GLOVES "gloves"
#define KEY_BOOTS "boots"

#endif /* JP */

#define MATCH_KEY(KEY) (!strncmp(ptr, KEY, sizeof(KEY)-1)\
     ? (prev_ptr = ptr, ptr += sizeof(KEY)-1, (' '==*ptr) ? ptr++ : 0, TRUE) : FALSE)

#ifdef JP
#define ADD_KEY(KEY) strcat(ptr, KEY)
#else
#define ADD_KEY(KEY) (strcat(ptr, KEY), strcat(ptr, " "))
#endif
#define ADD_KEY2(KEY) strcat(ptr, KEY)

#define ADD_FLG(FLG) (entry->flag[FLG / 32] |= (1L << (FLG % 32)))
#define REM_FLG(FLG) (entry->flag[FLG / 32] &= ~(1L << (FLG % 32)))
#define ADD_FLG2(FLG) (entry->flag[FLG / 32] |= (1L << (FLG % 32)), prev_flg = FLG)
#define IS_FLG(FLG) (entry->flag[FLG / 32] & (1L << (FLG % 32)))

#ifdef JP
	static char kanji_colon[] = "��";
#endif


/*
 * Reconstruct preference line from entry
 */
cptr autopick_line_from_entry(autopick_type *entry)
{
	char buf[1024];
	char *ptr;
	bool sepa_flag = TRUE;

	*buf = '\0';
	if (!(entry->action & DO_DISPLAY)) strcat(buf, "(");
	if (entry->action & DO_AUTODESTROY) strcat(buf, "!");
	if (entry->action & DONT_AUTOPICK) strcat(buf, "~");

	ptr = buf;

	if (IS_FLG(FLG_ALL)) ADD_KEY(KEY_ALL);
	if (IS_FLG(FLG_COLLECTING)) ADD_KEY(KEY_COLLECTING);
	if (IS_FLG(FLG_UNIDENTIFIED)) ADD_KEY(KEY_UNIDENTIFIED);
	if (IS_FLG(FLG_IDENTIFIED)) ADD_KEY(KEY_IDENTIFIED);
	if (IS_FLG(FLG_STAR_IDENTIFIED)) ADD_KEY(KEY_STAR_IDENTIFIED);
	if (IS_FLG(FLG_NAMELESS)) ADD_KEY(KEY_NAMELESS);
	if (IS_FLG(FLG_UNAWARE)) ADD_KEY(KEY_UNAWARE);
	if (IS_FLG(FLG_WORTHLESS)) ADD_KEY(KEY_WORTHLESS);
	if (IS_FLG(FLG_BOOSTED)) ADD_KEY(KEY_BOOSTED);

	if (IS_FLG(FLG_MORE_THAN))
	{
		ADD_KEY(KEY_MORE_THAN);
		strcat(ptr, format("%2d", entry->dice));
		ADD_KEY(KEY_DICE);
	}

	if (IS_FLG(FLG_WANTED)) ADD_KEY(KEY_WANTED);
	if (IS_FLG(FLG_UNIQUE)) ADD_KEY(KEY_UNIQUE);
	if (IS_FLG(FLG_HUMAN)) ADD_KEY(KEY_HUMAN);
	if (IS_FLG(FLG_UNREADABLE)) ADD_KEY(KEY_UNREADABLE);
	if (IS_FLG(FLG_REALM1)) ADD_KEY(KEY_REALM1);
	if (IS_FLG(FLG_REALM2)) ADD_KEY(KEY_REALM2);
	if (IS_FLG(FLG_FIRST)) ADD_KEY(KEY_FIRST);
	if (IS_FLG(FLG_SECOND)) ADD_KEY(KEY_SECOND);
	if (IS_FLG(FLG_THIRD)) ADD_KEY(KEY_THIRD);
	if (IS_FLG(FLG_FOURTH)) ADD_KEY(KEY_FOURTH);

	if (IS_FLG(FLG_ITEMS)) ADD_KEY2(KEY_ITEMS);
	else if (IS_FLG(FLG_ARTIFACTS)) ADD_KEY2(KEY_ARTIFACTS);
	else if (IS_FLG(FLG_WEAPONS)) ADD_KEY2(KEY_WEAPONS);
	else if (IS_FLG(FLG_ARMORS)) ADD_KEY2(KEY_ARMORS);
	else if (IS_FLG(FLG_MISSILES)) ADD_KEY2(KEY_MISSILES);
	else if (IS_FLG(FLG_DEVICES)) ADD_KEY2(KEY_DEVICES);
	else if (IS_FLG(FLG_LIGHTS)) ADD_KEY2(KEY_LIGHTS);
	else if (IS_FLG(FLG_JUNKS)) ADD_KEY2(KEY_JUNKS);
	else if (IS_FLG(FLG_SPELLBOOKS)) ADD_KEY2(KEY_SPELLBOOKS);
	else if (IS_FLG(FLG_HAFTED)) ADD_KEY2(KEY_HAFTED);
	else if (IS_FLG(FLG_SHIELDS)) ADD_KEY2(KEY_SHIELDS);
	else if (IS_FLG(FLG_BOWS)) ADD_KEY2(KEY_BOWS);
	else if (IS_FLG(FLG_RINGS)) ADD_KEY2(KEY_RINGS);
	else if (IS_FLG(FLG_AMULETS)) ADD_KEY2(KEY_AMULETS);
	else if (IS_FLG(FLG_SUITS)) ADD_KEY2(KEY_SUITS);
	else if (IS_FLG(FLG_CLOAKS)) ADD_KEY2(KEY_CLOAKS);
	else if (IS_FLG(FLG_HELMS)) ADD_KEY2(KEY_HELMS);
	else if (IS_FLG(FLG_GLOVES)) ADD_KEY2(KEY_GLOVES);
	else if (IS_FLG(FLG_BOOTS)) ADD_KEY2(KEY_BOOTS);
	else
		sepa_flag = FALSE;

	if (entry->name && *entry->name)
	{
		if (sepa_flag)
			strcat(buf, ":");
		strcat(buf, entry->name);
	}
	else
	{
		if (entry->flag[0] == 0L && entry->flag[0] == 0L)
			return NULL;
	}

	if (entry->insc)
	{
		strcat(buf, "#");
		strcat(buf, entry->insc);
	}

	return string_make(buf);
}

/*
 * A function to create new entry
 */
bool autopick_new_entry(autopick_type *entry, cptr str)
{
	cptr insc;
	int i;
	byte act = 0;
	char buf[1024];
	cptr prev_ptr, ptr;
	int prev_flg;

	if (str[1] == ':') switch (str[0])
	{
	case '?': case '%':
	case 'A': case 'P': case 'C':
		return FALSE;
	}

	entry->flag[0] = entry->flag[1] = 0L;
	entry->dice = 0;

	act = DO_AUTOPICK | DO_DISPLAY;
	while (1)
	{
		if (*str == '!')
		{
			act &= ~DO_AUTOPICK;
			act |= DO_AUTODESTROY;
			str++;
		}
		else if (*str == '~')
		{
			act &= ~DO_AUTOPICK;
			act |= DONT_AUTOPICK;
			str++;
		}
		else if (*str == '(')
		{
			act &= ~DO_DISPLAY;
			str++;
		}
		else
			break;
	}

	/* don't mind upper or lower case */
	insc = NULL;
	for (i = 0; *str; i++)
	{
		char c = *str++;
#ifdef JP
		if (iskanji(c))
		{
			buf[i++] = c;
			buf[i] = *str++;
			continue;
		}
#endif
		/* Auto-inscription? */
		if (c == '#')
		{
			buf[i] = '\0';
			insc = str;
			break;
		}

		if (isupper(c)) c = tolower(c);

		buf[i] = c;
	}
	buf[i] = '\0';
	
	/* Skip empty line */
	if (*buf == 0) return FALSE;

	/* Found flags */
	prev_ptr = ptr = buf;
	prev_flg = -1;
	if (MATCH_KEY(KEY_ALL)) ADD_FLG(FLG_ALL);
	if (MATCH_KEY(KEY_COLLECTING)) ADD_FLG(FLG_COLLECTING);
	if (MATCH_KEY(KEY_UNIDENTIFIED)) ADD_FLG(FLG_UNIDENTIFIED);
	if (MATCH_KEY(KEY_IDENTIFIED)) ADD_FLG(FLG_IDENTIFIED);
	if (MATCH_KEY(KEY_STAR_IDENTIFIED)) ADD_FLG(FLG_STAR_IDENTIFIED);
	if (MATCH_KEY(KEY_NAMELESS)) ADD_FLG(FLG_NAMELESS);
	if (MATCH_KEY(KEY_UNAWARE)) ADD_FLG(FLG_UNAWARE);
	if (MATCH_KEY(KEY_WORTHLESS)) ADD_FLG(FLG_WORTHLESS);
	if (MATCH_KEY(KEY_BOOSTED)) ADD_FLG(FLG_BOOSTED);

	/*** Weapons whic dd*ds is more than nn ***/
	if (MATCH_KEY(KEY_MORE_THAN))
	{
		if (isdigit(ptr[0]) && isdigit(ptr[1]))
		{
			entry->dice = (ptr[0] - '0') * 10 + (ptr[1] - '0');
			ptr += 2;
			(void)MATCH_KEY(KEY_DICE);
			ADD_FLG(FLG_MORE_THAN);
		}
		else
			ptr = prev_ptr;
	}

	if (MATCH_KEY(KEY_WANTED)) ADD_FLG(FLG_WANTED);
	if (MATCH_KEY(KEY_UNIQUE)) ADD_FLG(FLG_UNIQUE);
	if (MATCH_KEY(KEY_HUMAN)) ADD_FLG(FLG_HUMAN);
	if (MATCH_KEY(KEY_UNREADABLE)) ADD_FLG(FLG_UNREADABLE);
	if (MATCH_KEY(KEY_REALM1)) ADD_FLG(FLG_REALM1);
	if (MATCH_KEY(KEY_REALM2)) ADD_FLG(FLG_REALM2);
	if (MATCH_KEY(KEY_FIRST)) ADD_FLG(FLG_FIRST);
	if (MATCH_KEY(KEY_SECOND)) ADD_FLG(FLG_SECOND);
	if (MATCH_KEY(KEY_THIRD)) ADD_FLG(FLG_THIRD);
	if (MATCH_KEY(KEY_FOURTH)) ADD_FLG(FLG_FOURTH);

	/* Reset previous word location */
	prev_ptr = ptr;

	if (MATCH_KEY(KEY_ITEMS)) ADD_FLG2(FLG_ITEMS);
	else if (MATCH_KEY(KEY_ARTIFACTS)) ADD_FLG2(FLG_ARTIFACTS);
	else if (MATCH_KEY(KEY_WEAPONS)) ADD_FLG2(FLG_WEAPONS);
	else if (MATCH_KEY(KEY_ARMORS)) ADD_FLG2(FLG_ARMORS);
	else if (MATCH_KEY(KEY_MISSILES)) ADD_FLG2(FLG_MISSILES);
	else if (MATCH_KEY(KEY_DEVICES)) ADD_FLG2(FLG_DEVICES);
	else if (MATCH_KEY(KEY_LIGHTS)) ADD_FLG2(FLG_LIGHTS);
	else if (MATCH_KEY(KEY_JUNKS)) ADD_FLG2(FLG_JUNKS);
	else if (MATCH_KEY(KEY_SPELLBOOKS)) ADD_FLG2(FLG_SPELLBOOKS);
	else if (MATCH_KEY(KEY_HAFTED)) ADD_FLG2(FLG_HAFTED);
	else if (MATCH_KEY(KEY_SHIELDS)) ADD_FLG2(FLG_SHIELDS);
	else if (MATCH_KEY(KEY_BOWS)) ADD_FLG2(FLG_BOWS);
	else if (MATCH_KEY(KEY_RINGS)) ADD_FLG2(FLG_RINGS);
	else if (MATCH_KEY(KEY_AMULETS)) ADD_FLG2(FLG_AMULETS);
	else if (MATCH_KEY(KEY_SUITS)) ADD_FLG2(FLG_SUITS);
	else if (MATCH_KEY(KEY_CLOAKS)) ADD_FLG2(FLG_CLOAKS);
	else if (MATCH_KEY(KEY_HELMS)) ADD_FLG2(FLG_HELMS);
	else if (MATCH_KEY(KEY_GLOVES)) ADD_FLG2(FLG_GLOVES);
	else if (MATCH_KEY(KEY_BOOTS)) ADD_FLG2(FLG_BOOTS);

	/* Last 'keyword' must be at the correct location */
	if (*ptr == ':')
		ptr++;
	else if (*ptr == '\0')
		; /* nothing to do */
#ifdef JP
	else if (ptr[0] == kanji_colon[0] && ptr[1] == kanji_colon[1])
		ptr += 2;
#endif
	else
	{
		if (prev_flg != -1)
			entry->flag[prev_flg/32] &= ~(1L<< (prev_flg%32));
		ptr = prev_ptr;
	}
	entry->name = string_make(ptr);
	entry->action = act;
	entry->insc = string_make(insc);

	return TRUE;
}

/*
 * A function to delete entry
 */
void autopick_free_entry(autopick_type *entry)
{
	string_free(entry->name);
	string_free(entry->insc);
}

/*
 * A function for Auto-picker/destroyer
 * Examine whether the object matches to the list of keywords or not.
 */

int is_autopick(object_type *o_ptr)
{
	int i;
	char o_name[MAX_NLEN];

	if (o_ptr->tval == TV_GOLD) return -1;
	
	object_desc(o_name, o_ptr, FALSE, 3);

	/* Force to be lower case string */
	for (i = 0; o_name[i]; i++)
	{
#ifdef JP
		if (iskanji(o_name[i]))
			i++;
		else
#endif
		if (isupper(o_name[i]))
			o_name[i] = tolower(o_name[i]);
	}
	
	for (i=0; i < max_autopick; i++)
	{
		autopick_type *entry = &autopick_list[i];
		bool flag = FALSE;
		cptr ptr = autopick_list[i].name;

		/*** Unidentified ***/
		if (IS_FLG(FLG_UNIDENTIFIED)
		    && (object_known_p(o_ptr) || (o_ptr->ident & IDENT_SENSE)))
			continue;

		/*** Identified ***/
		if (IS_FLG(FLG_IDENTIFIED) && !object_known_p(o_ptr))
			continue;

		/*** *Identified* ***/
		if (IS_FLG(FLG_STAR_IDENTIFIED) &&
		    (!object_known_p(o_ptr) || !(o_ptr->ident & IDENT_MENTAL)))
			continue;

		/*** Nameless ***/
		if (IS_FLG(FLG_NAMELESS))
		{
			switch (o_ptr->tval)
			{
			case TV_WHISTLE:
			case TV_SHOT: case TV_ARROW: case TV_BOLT: case TV_BOW:
			case TV_DIGGING: case TV_HAFTED: case TV_POLEARM: case TV_SWORD: 
			case TV_BOOTS: case TV_GLOVES: case TV_HELM: case TV_CROWN:
			case TV_SHIELD: case TV_CLOAK:
			case TV_SOFT_ARMOR: case TV_HARD_ARMOR: case TV_DRAG_ARMOR:
			case TV_LITE: case TV_AMULET: case TV_RING: case TV_CARD:
				if ((!object_known_p(o_ptr) || o_ptr->inscription
				     || o_ptr->name1 || o_ptr->name2 || o_ptr->art_name))
					continue;
				break;
			default:
				/* don't match */
				continue;
			}
		}

		/*** Unaware items ***/
		if (IS_FLG(FLG_UNAWARE) && object_aware_p(o_ptr))
			continue;

		/*** Worthless items ***/
		if (IS_FLG(FLG_WORTHLESS) && object_value(o_ptr) > 0)
			continue;

		/*** Dice boosted (weapon of slaying) ***/
		if (IS_FLG(FLG_BOOSTED))
		{
			object_kind *k_ptr = &k_info[o_ptr->k_idx];
			
			switch( o_ptr->tval )
			{
			case TV_HAFTED:
			case TV_POLEARM:
			case TV_SWORD:
			case TV_DIGGING:
				if ((o_ptr->dd != k_ptr->dd) || (o_ptr->ds != k_ptr->ds))
					break;
				else
					continue;
			default:
				continue;
			}
		}

		/*** Weapons whic dd*ds is more than nn ***/
		if (IS_FLG(FLG_MORE_THAN))
		{
			if (o_ptr->dd * o_ptr->ds < entry->dice)
				continue;
		}
				
		/*** Wanted monster's corpse/skeletons ***/
		if (IS_FLG(FLG_WANTED) &&
		    (o_ptr->tval != TV_CORPSE || !object_is_shoukinkubi(o_ptr)))
			continue;

		/*** Unique monster's corpse/skeletons/statues ***/
		if (IS_FLG(FLG_UNIQUE) &&
		    ((o_ptr->tval != TV_CORPSE && o_ptr->tval != TV_STATUE) ||
		    !(r_info[o_ptr->pval].flags1 & RF1_UNIQUE)))
			continue;

		/*** Human corpse/skeletons (for Daemon magic) ***/
		if (IS_FLG(FLG_HUMAN) &&
		    (o_ptr->tval != TV_CORPSE ||
		    !strchr("pht", r_info[o_ptr->pval].d_char)))
			continue;

		/*** Unreadable spellbooks ***/
		if (IS_FLG(FLG_UNREADABLE) &&
		    (o_ptr->tval < TV_LIFE_BOOK ||
		    check_book_realm(o_ptr->tval, o_ptr->sval)))
			continue;

		/*** First realm spellbooks ***/
		if (IS_FLG(FLG_REALM1) && 
		    (REALM1_BOOK != o_ptr->tval ||
		    p_ptr->pclass == CLASS_SORCERER ||
		    p_ptr->pclass == CLASS_RED_MAGE))
			continue;

		/*** Second realm spellbooks ***/
		if (IS_FLG(FLG_REALM2) &&
		    (REALM2_BOOK != o_ptr->tval ||
		    p_ptr->pclass == CLASS_SORCERER ||
		    p_ptr->pclass == CLASS_RED_MAGE))
			continue;

		/*** First rank spellbooks ***/
		if (IS_FLG(FLG_FIRST) &&
		    (o_ptr->tval < TV_LIFE_BOOK || 0 != o_ptr->sval))
			continue;

		/*** Second rank spellbooks ***/
		if (IS_FLG(FLG_SECOND) &&
		    (o_ptr->tval < TV_LIFE_BOOK || 1 != o_ptr->sval))
			continue;

		/*** Third rank spellbooks ***/
		if (IS_FLG(FLG_THIRD) && 
		    (o_ptr->tval < TV_LIFE_BOOK || 2 != o_ptr->sval))
			continue;

		/*** Fourth rank spellbooks ***/
		if (IS_FLG(FLG_FOURTH) &&
		    (o_ptr->tval < TV_LIFE_BOOK || 3 != o_ptr->sval))
			continue;

		/*** Items ***/
		if (IS_FLG(FLG_ARTIFACTS))
		{
			if (!(object_known_p(o_ptr)
			      && (artifact_p(o_ptr) || o_ptr->art_name)))
				continue;
		}
		else if (IS_FLG(FLG_WEAPONS))
		{
			switch(o_ptr->tval)
			{
			case TV_BOW: case TV_HAFTED: case TV_POLEARM:
			case TV_SWORD: case TV_DIGGING:
				break;
			default: continue;
			}
		}
		else if (IS_FLG(FLG_ARMORS))
		{
			switch(o_ptr->tval)
			{
			case TV_BOOTS: case TV_GLOVES: case TV_CLOAK: case TV_CROWN:
			case TV_HELM: case TV_SHIELD: case TV_SOFT_ARMOR:
			case TV_HARD_ARMOR: case TV_DRAG_ARMOR:
				break;
			default: continue;
			}
		}
		else if (IS_FLG(FLG_MISSILES))
		{
			switch(o_ptr->tval)
			{
			case TV_SHOT: case TV_BOLT: case TV_ARROW:
				break;
			default: continue;
			}
		}
		else if (IS_FLG(FLG_DEVICES))
		{
			switch(o_ptr->tval)
			{
			case TV_SCROLL: case TV_STAFF: case TV_WAND: case TV_ROD:
				break;
			default: continue;
			}
		}
		else if (IS_FLG(FLG_LIGHTS))
		{
			if (!(o_ptr->tval == TV_LITE))
				continue;
		}
		else if (IS_FLG(FLG_JUNKS))
		{
			switch(o_ptr->tval)
			{
			case TV_SKELETON: case TV_BOTTLE:
			case TV_JUNK: case TV_STATUE:
				break;
			default: continue;
			}
		}
		else if (IS_FLG(FLG_SPELLBOOKS))
		{
			if (!(o_ptr->tval >= TV_LIFE_BOOK))
				continue;
		}
		else if (IS_FLG(FLG_HAFTED))
		{
			if (!(o_ptr->tval == TV_HAFTED))
				continue;
		}
		else if (IS_FLG(FLG_SHIELDS))
		{
			if (!(o_ptr->tval == TV_SHIELD))
				continue;
		}
		else if (IS_FLG(FLG_BOWS))
		{
			if (!(o_ptr->tval == TV_BOW))
				continue;
		}
		else if (IS_FLG(FLG_RINGS))
		{
			if (!(o_ptr->tval == TV_RING))
				continue;
		}
		else if (IS_FLG(FLG_AMULETS))
		{
			if (!(o_ptr->tval == TV_AMULET))
				continue;
		}
		else if (IS_FLG(FLG_SUITS))
		{
			if (!(o_ptr->tval == TV_DRAG_ARMOR ||
			      o_ptr->tval == TV_HARD_ARMOR ||
			      o_ptr->tval == TV_SOFT_ARMOR))
				continue;
		}
		else if (IS_FLG(FLG_CLOAKS))
		{
			if (!(o_ptr->tval == TV_CLOAK))
				continue;
		}
		else if (IS_FLG(FLG_HELMS))
		{
			if (!(o_ptr->tval == TV_CROWN || o_ptr->tval == TV_HELM))
				continue;
		}
		else if (IS_FLG(FLG_GLOVES))
		{
			if (!(o_ptr->tval == TV_GLOVES))
				continue;
		}
		else if (IS_FLG(FLG_BOOTS))
		{
			if (!(o_ptr->tval == TV_BOOTS))
				continue;
		}


		if (*ptr == '^')
		{
			ptr++;
			if (!strncmp(o_name, ptr, strlen(ptr)))
				flag = TRUE;
		}
		else
#ifdef JP
			if (strstr_j(o_name, ptr))
#else
			if (strstr(o_name, ptr))
#endif
		{
			flag = TRUE;
		}

		if (flag)
		{
			int j;
			if (!IS_FLG(FLG_COLLECTING))
				return i;
			/* Check if there is a same item */
			for (j = 0; j < INVEN_PACK; j++)
			{
				if (object_similar(&inventory[j], o_ptr))
					return i;
			}
		}
	}/* for */

	return -1;
}


/*
 * Describe which kind of object is Auto-picked/destroyed
 */
static void describe_autopick(char *buff, autopick_type *entry)
{
	cptr str = entry->name;
	byte act = entry->action;
	cptr insc = entry->insc;
	int i;

	bool top = FALSE;

#ifdef JP
	cptr before_str[20], body_str;
	int before_n = 0;

	body_str = "�����ƥ�";

	/*** Collecting items ***/
	/*** Which can be absorbed into a slot as a bundle ***/
	if (IS_FLG(FLG_COLLECTING))
		before_str[before_n++] = "������Ǵ��˻��äƤ��륹���åȤˤޤȤ����";
	
	/*** Unidentified ***/
	if (IS_FLG(FLG_UNIDENTIFIED))
		before_str[before_n++] = "̤�����";

	/*** Identified ***/
	if (IS_FLG(FLG_IDENTIFIED))
		before_str[before_n++] = "����Ѥߤ�";

	/*** *Identified* ***/
	if (IS_FLG(FLG_STAR_IDENTIFIED))
		before_str[before_n++] = "�����˴���Ѥߤ�";

	/*** Nameless ***/
	if (IS_FLG(FLG_NAMELESS))
	{
		before_str[before_n++] = "�����Ǥ⥢���ƥ��ե����ȤǤ�ʤ�";
		body_str = "����";
	}

	/*** Unaware items ***/
	if (IS_FLG(FLG_UNAWARE))
		before_str[before_n++] = "̤����Ǥ��θ��̤�Ƚ�����Ƥ��ʤ�";

	/*** Worthless items ***/
	if (IS_FLG(FLG_WORTHLESS))
		before_str[before_n++] = "Ź��̵���ͤ�Ƚ�ꤵ���";

	/*** Dice boosted (weapon of slaying) ***/
	if (IS_FLG(FLG_BOOSTED))
	{
		before_str[before_n++] = "���᡼�����������̾����礭��";
		body_str = "���";
	}

	/*** Weapons whose dd*ds is more than nn ***/
	if (IS_FLG(FLG_MORE_THAN))
	{
		static char more_than_desc_str[] = "___";
		before_str[before_n++] = "���᡼���������κ����ͤ�";
		body_str = "���";
			
		sprintf(more_than_desc_str,"%2d", entry->dice);
		before_str[before_n++] = more_than_desc_str;
		before_str[before_n++] = "�ʾ��";
	}

	/*** Wanted monster's corpse/skeletons ***/
	if (IS_FLG(FLG_WANTED))
	{
		before_str[before_n++] = "�ϥ󥿡���̳��Ǿ޶��Ȥ���Ƥ���";
		body_str = "���Τ��";
	}

	/*** Human corpse/skeletons (for Daemon magic) ***/
	if (IS_FLG(FLG_HUMAN))
	{
		before_str[before_n++] = "������ˡ�ǻȤ�����οʹ֤�ҥ塼�ޥΥ��ɤ�";
		body_str = "���Τ��";
	}

	/*** Unique monster's corpse/skeletons/statues ***/
	if (IS_FLG(FLG_UNIQUE))
	{
		before_str[before_n++] = "��ˡ�����󥹥�����";
		body_str = "���Τ��";
	}

	/*** Unreadable spellbooks ***/
	if (IS_FLG(FLG_UNREADABLE))
	{
		before_str[before_n++] = "�ΰ褬�ۤʤ�٤ˤ��ʤ��ˤ��ɤ�ʤ�";
		body_str = "��ˡ��";
	}

	/*** First realm spellbooks ***/
	if (IS_FLG(FLG_REALM1))
	{
		before_str[before_n++] = "����ΰ��";
		body_str = "��ˡ��";
	}

	/*** Second realm spellbooks ***/
	if (IS_FLG(FLG_REALM2))
	{
		before_str[before_n++] = "�����ΰ��";
		body_str = "��ˡ��";
	}

	/*** First rank spellbooks ***/
	if (IS_FLG(FLG_FIRST))
	{
		before_str[before_n++] = "��4�������1���ܤ�";
		body_str = "��ˡ��";
	}

	/*** Second rank spellbooks ***/
	if (IS_FLG(FLG_SECOND))
	{
		before_str[before_n++] = "��4�������2���ܤ�";
		body_str = "��ˡ��";
	}

	/*** Third rank spellbooks ***/
	if (IS_FLG(FLG_THIRD))
	{
		before_str[before_n++] = "��4�������3���ܤ�";
		body_str = "��ˡ��";
	}

	/*** Fourth rank spellbooks ***/
	if (IS_FLG(FLG_FOURTH))
	{
		before_str[before_n++] = "��4�������4���ܤ�";
		body_str = "��ˡ��";
	}

	/*** Items ***/
	if (IS_FLG(FLG_ITEMS))
		; /* Nothing to do */
	else if (IS_FLG(FLG_ARTIFACTS))
		body_str = "�����ƥ��ե�����";
	else if (IS_FLG(FLG_WEAPONS))
		body_str = "���";
	else if (IS_FLG(FLG_ARMORS))
		body_str = "�ɶ�";
	else if (IS_FLG(FLG_MISSILES))
		body_str = "�Ƥ���䥯�����ܥ�����";
	else if (IS_FLG(FLG_DEVICES))
		body_str = "��ʪ����ˡ��������å�";
	else if (IS_FLG(FLG_LIGHTS))
		body_str = "�����ѤΥ����ƥ�";
	else if (IS_FLG(FLG_JUNKS))
		body_str = "�ޤ줿�����Υ��饯��";
	else if (IS_FLG(FLG_SPELLBOOKS))
		body_str = "��ˡ��";
	else if (IS_FLG(FLG_HAFTED))
		body_str = "�ߴ�";
	else if (IS_FLG(FLG_SHIELDS))
		body_str = "��";
	else if (IS_FLG(FLG_BOWS))
		body_str = "����󥰤�ݤ䥯�����ܥ�";
	else if (IS_FLG(FLG_RINGS))
		body_str = "����";
	else if (IS_FLG(FLG_AMULETS))
		body_str = "���ߥ��å�";
	else if (IS_FLG(FLG_SUITS))
		body_str = "��";
	else if (IS_FLG(FLG_CLOAKS))
		body_str = "��������";
	else if (IS_FLG(FLG_HELMS))
		body_str = "�إ��åȤ䴧";
	else if (IS_FLG(FLG_GLOVES))
		body_str = "����";
	else if (IS_FLG(FLG_BOOTS))
		body_str = "�֡���";

	*buff = '\0';
	if (!before_n) 
		strcat(buff, "���Ƥ�");
	else for (i = 0; i < before_n && before_str[i]; i++)
		strcat(buff, before_str[i]);

	strcat(buff, body_str);

	if (*str)
	{
		if (*str == '^')
		{
			str++;
			top = TRUE;
		}

		strcat(buff, "�ǡ�̾������");
		strncat(buff, str, 80);
		if (top)
			strcat(buff, "�פǻϤޤ���");
		else
			strcat(buff, "�פ�ޤ���");
	}

	if (insc)
		strncat(buff, format("�ˡ�%s�פȹ���", insc), 80);
	else
		strcat(buff, "��");

	if (act & DONT_AUTOPICK)
		strcat(buff, "���֤��롣");
	else if (act & DO_AUTODESTROY)
		strcat(buff, "�˲����롣");
	else
		strcat(buff, "������");

	if (act & DO_DISPLAY)
	{
		if (act & DONT_AUTOPICK)
			strcat(buff, "���Υޥå�('M')��'N'�򲡤����Ȥ���ɽ�����롣");
		else if (act & DO_AUTODESTROY)
			strcat(buff, "���Υޥå�('M')��'K'�򲡤����Ȥ���ɽ�����롣");
		else
			strcat(buff, "���Υޥå�('M')��'M'�򲡤����Ȥ���ɽ�����롣");
	}
	else
		strcat(buff, "���Υޥåפˤ�ɽ�����ʤ�");

#else /* JP */

	cptr before_str[20], after_str[20], which_str[20], whose_str[20], body_str;
	int before_n = 0, after_n = 0, which_n = 0, whose_n = 0;

	body_str = "items";

	/*** Collecting items ***/
	/*** Which can be absorbed into a slot as a bundle ***/
	if (IS_FLG(FLG_COLLECTING))
		which_str[which_n++] = "can be absorbed into an existing inventory slot";
	
	/*** Unidentified ***/
	if (IS_FLG(FLG_UNIDENTIFIED))
		before_str[before_n++] = "unidentified";

	/*** Identified ***/
	if (IS_FLG(FLG_IDENTIFIED))
		before_str[before_n++] = "identified";

	/*** *Identified* ***/
	if (IS_FLG(FLG_STAR_IDENTIFIED))
		before_str[before_n++] = "fully identified";

	/*** Nameless ***/
	if (IS_FLG(FLG_NAMELESS))
	{
		body_str = "equipment";
		which_str[which_n++] = "is neither ego-item nor artifact";
	}

	/*** Unaware items ***/
	if (IS_FLG(FLG_UNAWARE))
	{
		before_str[before_n++] = "unidentified";
		whose_str[whose_n++] = "basic abilities are not known";
	}

	/*** Worthless items ***/
	if (IS_FLG(FLG_WORTHLESS))
	{
		before_str[before_n++] = "worthless";
		which_str[which_n++] = "can not be sold at stores";
	}

	/*** Dice boosted (weapon of slaying) ***/
	if (IS_FLG(FLG_BOOSTED))
	{
		body_str = "weapons";
		whose_str[whose_n++] = "damage dice is bigger than normal";
	}

	/*** Weapons whic dd*ds is more than nn ***/
	if (IS_FLG(FLG_MORE_THAN))
	{
		static char more_than_desc_str[] =
			"maximum damage from dice is bigger than __";
		body_str = "weapons";
			
		sprintf(more_than_desc_str + sizeof(more_than_desc_str) - 3,
			"%2d", entry->dice);
		whose_str[whose_n++] = more_than_desc_str;
	}

	/*** Wanted monster's corpse/skeletons ***/
	if (IS_FLG(FLG_WANTED))
	{
		body_str = "corpse or skeletons";
		which_str[which_n++] = "is wanted at the Hunter's Office";
	}

	/*** Human corpse/skeletons (for Daemon magic) ***/
	if (IS_FLG(FLG_HUMAN))
	{
		before_str[before_n++] = "humanoid";
		body_str = "corpse or skeletons";
		which_str[which_n++] = "can be used for Daemon magic";
	}

	/*** Unique monster's corpse/skeletons/statues ***/
	if (IS_FLG(FLG_UNIQUE))
	{
		before_str[before_n++] = "unique monster's";
		body_str = "corpse or skeletons";
	}

	/*** Unreadable spellbooks ***/
	if (IS_FLG(FLG_UNREADABLE))
	{
		body_str = "spellbooks";
		after_str[after_n++] = "of different realms from yours";
	}

	/*** First realm spellbooks ***/
	if (IS_FLG(FLG_REALM1))
	{
		body_str = "spellbooks";
		after_str[after_n++] = "of your first realm";
	}

	/*** Second realm spellbooks ***/
	if (IS_FLG(FLG_REALM2))
	{
		body_str = "spellbooks";
		after_str[after_n++] = "of your second realm";
	}

	/*** First rank spellbooks ***/
	if (IS_FLG(FLG_FIRST))
	{
		before_str[before_n++] = "first one of four ";
		body_str = "spellbooks";
	}

	/*** Second rank spellbooks ***/
	if (IS_FLG(FLG_SECOND))
	{
		before_str[before_n++] = "second one of four ";
		body_str = "spellbooks";
	}

	/*** Third rank spellbooks ***/
	if (IS_FLG(FLG_THIRD))
	{
		before_str[before_n++] = "third one of four ";
		body_str = "spellbooks";
	}

	/*** Fourth rank spellbooks ***/
	if (IS_FLG(FLG_FOURTH))
	{
		before_str[before_n++] = "fourth one of four ";
		body_str = "spellbooks";
	}

	/*** Items ***/
	if (IS_FLG(FLG_ITEMS))
		; /* Nothing to do */
	else if (IS_FLG(FLG_WEAPONS))
		body_str = "weapons";
	else if (IS_FLG(FLG_ARMORS))
		body_str = "armors";
	else if (IS_FLG(FLG_MISSILES))
		body_str = "shots, arrows or crossbow bolts";
	else if (IS_FLG(FLG_DEVICES))
		body_str = "scrolls, wands, staves or rods";
	else if (IS_FLG(FLG_LIGHTS))
		body_str = "light sources";
	else if (IS_FLG(FLG_JUNKS))
		body_str = "junk such as broken sticks";
	else if (IS_FLG(FLG_SPELLBOOKS))
		body_str = "spellbooks";
	else if (IS_FLG(FLG_HAFTED))
		body_str = "hafted weapons";
	else if (IS_FLG(FLG_SHIELDS))
		body_str = "shields";
	else if (IS_FLG(FLG_BOWS))
		body_str = "slings, bows or crossbows";
	else if (IS_FLG(FLG_RINGS))
		body_str = "rings";
	else if (IS_FLG(FLG_AMULETS))
		body_str = "amulets";
	else if (IS_FLG(FLG_SUITS))
		body_str = "body armors";
	else if (IS_FLG(FLG_CLOAKS))
		body_str = "cloaks";
	else if (IS_FLG(FLG_HELMS))
		body_str = "helms or crowns";
	else if (IS_FLG(FLG_GLOVES))
		body_str = "gloves";
	else if (IS_FLG(FLG_BOOTS))
		body_str = "boots";

	/* Prepare a string for item name */
	if (*str)
	{
		if (*str == '^')
		{
			str++;
			top = TRUE;
			whose_str[whose_n++] = "name is beginning with \"";
		}
		else
			which_str[which_n++] = "have \"";
	}


	/* Describe action flag */
	if (act & DONT_AUTOPICK)
		strcpy(buff, "Leave on floor ");
	else if (act & DO_AUTODESTROY)
		strcpy(buff, "Destroy ");
	else
		strcpy(buff, "Pickup ");

	/* Auto-insctiption */
	if (insc)
		strncat(buff, format("and inscribe \"%s\" on ", insc), 80);

	/* Adjective */
	if (!before_n) 
		strcat(buff, "all ");
	else for (i = 0; i < before_n && before_str[i]; i++)
	{
		strcat(buff, before_str[i]);
		strcat(buff, " ");
	}

	/* Item class */
	strcat(buff, body_str);

	/* of ... */
	for (i = 0; i < after_n && after_str[i]; i++)
	{
		strcat(buff, " ");
		strcat(buff, after_str[i]);
	}

	/* which ... */
	for (i = 0; i < whose_n && whose_str[i]; i++)
	{
		if (i == 0)
			strcat(buff, " whose ");
		else
			strcat(buff, ", and ");

		strcat(buff, whose_str[i]);
	}

	/* Item name ; whose name is beginning with "str" */
	if (*str && top)
	{
		strcat(buff, str);
		strcat(buff, "\"");
	}

	/* whose ..., and which .... */
	if (whose_n && which_n)
		strcat(buff, ", and ");

	/* which ... */
	for (i = 0; i < which_n && which_str[i]; i++)
	{
		if (i == 0)
			strcat(buff, " which ");
		else
			strcat(buff, ", and ");

		strcat(buff, which_str[i]);
	}

	/* Item name ; which have "str" as part of its name */
	if (*str && !top)
	{
		strncat(buff, str, 80);
		strcat(buff, "\" as part of its name");
	}
	strcat(buff, ".");

	/* Describe whether it will be displayed on the full map or not */
	if (act & DO_DISPLAY)
	{
		if (act & DONT_AUTOPICK)
			strcat(buff, "  Display these items when you press 'N' in the full map('M').");
		else if (act & DO_AUTODESTROY)
			strcat(buff, "  Display these items when you press 'K' in the full map('M').");
		else
			strcat(buff, "  Display these items when you press 'M' in the full map('M').");
	}
	else
		strcat(buff, " Not displayed in the full map.");
#endif /* JP */

}


#define MAX_LINES 3000

/*
 * Read whole lines of a file to memory
 */
static cptr *read_text_lines(cptr filename, bool user)
{
	cptr *lines_list = NULL;
	FILE *fff;

	int lines = 0;
	char buf[1024];

	if (user)
	{
		/* Hack -- drop permissions */
		safe_setuid_drop();
		path_build(buf, 1024, ANGBAND_DIR_USER, filename);
	}
	else
	{
		path_build(buf, 1024, ANGBAND_DIR_PREF, filename);
	}
	
	/* Open the file */
	fff = my_fopen(buf, "r");

	if (fff)
	{
		/* Allocate list of pointers */
		C_MAKE(lines_list, MAX_LINES, cptr);

		/* Parse it */
		while (0 == my_fgets(fff, buf, 1024))
		{
			lines_list[lines++] = string_make(buf);
			if (lines >= MAX_LINES - 1) break;
		}
		if (lines == 0)
			lines_list[0] = string_make("");

		my_fclose(fff);
	}

	/* Grab priv's */
	safe_setuid_grab();

	if (!fff) return NULL;
	return lines_list;
}

static cptr *read_pickpref_text_lines()
{
	char buf[1024];
	cptr *lines_list;

#ifdef JP
	sprintf(buf, "picktype-%s.prf", player_name);
#else
	sprintf(buf, "pickpref-%s.prf", player_name);
#endif
	lines_list = read_text_lines(buf, TRUE);

	if (!lines_list)
	{
#ifdef JP
		lines_list = read_text_lines("picktype.prf", TRUE);
#else
		lines_list = read_text_lines("pickpref.prf", TRUE);
#endif
	}

	if (!lines_list)
	{
#ifdef JP
		lines_list = read_text_lines("picktype.prf", FALSE);
#else
		lines_list = read_text_lines("pickpref.prf", FALSE);
#endif
	}

	if (!lines_list)
	{
		/* Allocate list of pointers */
		C_MAKE(lines_list, MAX_LINES, cptr);
		lines_list[0] = string_make("");
	}
	return lines_list;
}

/*
 * Write whole lines of memory to a file.
 */
static bool write_text_lines(cptr filename, cptr *lines_list)
{
	FILE *fff;

	int lines = 0;
	char buf[1024];

	/* Hack -- drop permissions */
	safe_setuid_drop();

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, filename);
	
	/* Open the file */
	fff = my_fopen(buf, "w");
	if (fff)
	{
		for (lines = 0; lines_list[lines]; lines++)
			my_fputs(fff, lines_list[lines], 1024);

		my_fclose(fff);
	}

	/* Grab priv's */
	safe_setuid_grab();

	if (!fff) return FALSE;
	return TRUE;
}


/*
 * Free memory of lines_list.
 */
static void free_text_lines(cptr *lines_list)
{
	int lines;

	for (lines = 0; lines_list[lines]; lines++)
		string_free(lines_list[lines]);

	/* free list of pointers */
	C_FREE((char **)lines_list, MAX_LINES, char *);
}


/*
 * Insert string
 */
static void insert_string(cptr *lines_list, cptr str, int x, int y)
{
	char buf[1024];
	int i, j;

	for (i = j = 0; lines_list[y][i] && i < x; i++)
		buf[j++] = lines_list[y][i];

	while (*str) buf[j++] = *str++;

	for (; lines_list[y][i]; i++)
		buf[j++] = lines_list[y][i];
	buf[j] = '\0';
	string_free(lines_list[y]);
	lines_list[y] = string_make(buf);
}

/*
 * Delete n letters
 */
static void delete_string(cptr *lines_list, int n, int x, int y)
{
	int i, j;
	char buf[1024];

	for (i = j = 0; lines_list[y][i] && i < x; i++)
	{
#ifdef JP
		if (iskanji(lines_list[y][i]))
			buf[j++] = lines_list[y][i++];
#endif
		buf[j++] = lines_list[y][i];
	}
	i += n;

	for (; lines_list[y][i]; i++)
		buf[j++] = lines_list[y][i];
	buf[j] = '\0';
	string_free(lines_list[y]);
	lines_list[y] = string_make(buf);
}


/*
 * Delete or insert string
 */
void toggle_string(cptr *lines_list, int flg, int y)
{
	autopick_type an_entry, *entry = &an_entry;

	if (!autopick_new_entry(entry, lines_list[y]))
		return;

	string_free(lines_list[y]);
	if (IS_FLG(flg)) 
		REM_FLG(flg);
	else
		ADD_FLG(flg);

	lines_list[y] = autopick_line_from_entry(entry);
}

/*
 * Insert return code and split the line
 */
static bool insert_return_code(cptr *lines_list, int cx, int cy)
{
	char buf[1024];
	int i, j, k;

	for (k = 0; lines_list[k]; k++)
		/* count number of lines */ ;

	if (k >= MAX_LINES - 2) return FALSE;
	k--;

	/* Move down lines */
	for (; cy < k; k--)
		lines_list[k+1] = lines_list[k];

	/* Split current line */
	for (i = j = 0; lines_list[cy][i] && i < cx; i++)
	{
#ifdef JP
		if (iskanji(lines_list[cy][i]))
			buf[j++] = lines_list[cy][i++];
#endif
		buf[j++] = lines_list[cy][i];
	}
	buf[j] = '\0';
	lines_list[cy+1] = string_make(&lines_list[cy][i]);
	string_free(lines_list[cy]);
	lines_list[cy] = string_make(buf);
	return TRUE;
}


/*
 * Get auto-picker entry from o_ptr.
 */
void autopick_entry_from_object(autopick_type *entry, object_type *o_ptr)
{
	char o_name[MAX_NLEN];
	object_desc(o_name, o_ptr, FALSE, 0);

	entry->name = string_make(o_name);
	entry->insc = string_make(quark_str(o_ptr->inscription));
	entry->action = DO_AUTOPICK | DO_DISPLAY;
	entry->flag[0] = entry->flag[1] = 0L;
	entry->dice = 0;

	if (!object_aware_p(o_ptr))
		ADD_FLG(FLG_UNAWARE);
	if (object_value(o_ptr) <= 0)
		ADD_FLG(FLG_WORTHLESS);

	switch(o_ptr->tval)
	{
		object_kind *k_ptr; 
	case TV_HAFTED: case TV_POLEARM: case TV_SWORD: case TV_DIGGING:
		k_ptr = &k_info[o_ptr->k_idx];
		if ((o_ptr->dd != k_ptr->dd) || (o_ptr->ds != k_ptr->ds))
			ADD_FLG(FLG_BOOSTED);
	}

	if (o_ptr->tval == TV_CORPSE && object_is_shoukinkubi(o_ptr))
	{
		REM_FLG(FLG_WORTHLESS);
		ADD_FLG(FLG_WANTED);
	}

	if ((o_ptr->tval == TV_CORPSE || o_ptr->tval == TV_STATUE)
	    && (r_info[o_ptr->pval].flags1 & RF1_UNIQUE))
	{
		REM_FLG(FLG_WORTHLESS);
		ADD_FLG(FLG_UNIQUE);
	}

	if (o_ptr->tval == TV_CORPSE && strchr("pht", r_info[o_ptr->pval].d_char))
	{
		REM_FLG(FLG_WORTHLESS);
		ADD_FLG(FLG_HUMAN);
	}

	if (o_ptr->tval >= TV_LIFE_BOOK &&
	    !check_book_realm(o_ptr->tval, o_ptr->sval))
		ADD_FLG(FLG_UNREADABLE);

	if (REALM1_BOOK == o_ptr->tval &&
	    p_ptr->pclass != CLASS_SORCERER &&
	    p_ptr->pclass != CLASS_RED_MAGE)
		ADD_FLG(FLG_REALM1);

	if (REALM2_BOOK == o_ptr->tval &&
	    p_ptr->pclass != CLASS_SORCERER &&
	    p_ptr->pclass != CLASS_RED_MAGE)
		ADD_FLG(FLG_REALM2);

	if (o_ptr->tval >= TV_LIFE_BOOK && 0 == o_ptr->sval)
		ADD_FLG(FLG_FIRST);
	if (o_ptr->tval >= TV_LIFE_BOOK && 1 == o_ptr->sval)
		ADD_FLG(FLG_SECOND);
	if (o_ptr->tval >= TV_LIFE_BOOK && 2 == o_ptr->sval)
		ADD_FLG(FLG_THIRD);
	if (o_ptr->tval >= TV_LIFE_BOOK && 3 == o_ptr->sval)
		ADD_FLG(FLG_FOURTH);

	if (object_known_p(o_ptr) && (artifact_p(o_ptr) || o_ptr->art_name))
		ADD_FLG(FLG_ARTIFACTS);
	
	else if (o_ptr->tval == TV_SHOT || o_ptr->tval == TV_BOLT
		 || o_ptr->tval == TV_ARROW)
		ADD_FLG(FLG_MISSILES);
	else if (o_ptr->tval == TV_SCROLL || o_ptr->tval == TV_STAFF
		 || o_ptr->tval == TV_WAND || o_ptr->tval == TV_ROD)
		ADD_FLG(FLG_DEVICES);
	else if (o_ptr->tval == TV_LITE)
		ADD_FLG(FLG_LIGHTS);
	else if (o_ptr->tval == TV_SKELETON || o_ptr->tval == TV_BOTTLE
		 || o_ptr->tval == TV_JUNK || o_ptr->tval == TV_STATUE)
		ADD_FLG(FLG_JUNKS);
	else if (o_ptr->tval >= TV_LIFE_BOOK)
		ADD_FLG(FLG_SPELLBOOKS);
	else if (o_ptr->tval == TV_HAFTED)
		ADD_FLG(FLG_HAFTED);
	else if (o_ptr->tval == TV_POLEARM || o_ptr->tval == TV_SWORD
		 || o_ptr->tval == TV_DIGGING)
		ADD_FLG(FLG_WEAPONS);
	else if (o_ptr->tval == TV_SHIELD)
		ADD_FLG(FLG_SHIELDS);
	else if (o_ptr->tval == TV_BOW)
		ADD_FLG(FLG_BOWS);
	else if (o_ptr->tval == TV_RING)
		ADD_FLG(FLG_RINGS);
	else if (o_ptr->tval == TV_AMULET)
		ADD_FLG(FLG_AMULETS);
	else if (o_ptr->tval == TV_DRAG_ARMOR || o_ptr->tval == TV_HARD_ARMOR ||
		 o_ptr->tval == TV_SOFT_ARMOR)
		ADD_FLG(FLG_SUITS);
	else if (o_ptr->tval == TV_CLOAK)
		ADD_FLG(FLG_CLOAKS);
	else if (o_ptr->tval == TV_HELM)
		ADD_FLG(FLG_HELMS);
	else if (o_ptr->tval == TV_GLOVES)
		ADD_FLG(FLG_GLOVES);
	else if (o_ptr->tval == TV_BOOTS)
		ADD_FLG(FLG_BOOTS);

	return;
}

/*
 * Choose an item and get auto-picker entry from it.
 */
static bool entry_from_object(autopick_type *entry)
{
	int item;
	object_type *o_ptr;
	cptr q, s;

	/* Get an item */
#ifdef JP
	q = "�ɤΥ����ƥ����Ͽ���ޤ���? ";
	s = "�����ƥ����äƤ��ʤ���";
#else
	q = "Entry which item? ";
	s = "You have nothing to entry.";
#endif
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR | USE_EQUIP))) return FALSE;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	autopick_entry_from_object(entry, o_ptr);
	return TRUE;
}


/*
 * Description of control commands
 */

#define WID_DESC 31

static cptr ctrl_command_desc[] =
{
#ifdef JP
#define LAST_DESTROYED 6
	"^P ^N ^B ^F �岼�����˰�ư",
	"^A ^E �Ԥ���Ƭ����ü",
	"^Q ����/���ޥ�ɥ⡼���ڤ��ؤ�",
	"^R �ѹ������Ƽ��ä��Ƹ����᤹",
        "------------------------------------",
        "^I ����ʪ/������������",
	"^L",
	"^K �������뤫�齪ü�ޤǺ��",
	"^Y ���(^K)�����Ԥ�����",
	"^C ��²�����Ȥξ�Ｐ������",
        "------------------------------------",
	"^S �ѹ� (!�˲�/~����/����)",
	"^G \"(\" ���Υޥåפ�ɽ�����ʤ�",
	"^O \"#\" ��ư���",
        "------------------------------------",
	"^U ̤����/̤Ƚ��/����/*����*",
	"^W \"̵���ͤ�\"",
	"^X \"̵�ä�\"",
	"^Z \"�������\"",
	NULL
#else
#define LAST_DESTROYED 6
	"^P ^N ^B ^F Move Cursor",
	"^A ^E Beginning and End of Line",
	"^Q Toggle Insert/Command mode",
	"^R Revert to Original File",
        "------------------------------------",
        "^I Object in Inventry/Equipment",
	"^L",
	"^K Kill Rest of Line",
	"^Y Insert killed(^K) text",
	"^C Insert conditional expression",
        "------------------------------------",
	"^S Toggle(!Destroy/~Leave/Pick)",
	"^G \"(\" No display in the 'M'ap",
	"^O \"#\" Auto-Inscribe",
        "------------------------------------",
	"^U Toggle 'identified' state",
	"^W \"worthless\"",
	"^X \"nameless\"",
	"^Z \"collecting\"",
	NULL
#endif
};


#define MAX_YANK 1024

/*
 * In-game editor of Object Auto-picker/Destoryer
 */
void do_cmd_edit_autopick()
{
	cptr last_destroyed;
	char last_destroyed_command[WID_DESC+3];
	char yank_buf[MAX_YANK];
	char classrace[80];
	autopick_type an_entry, *entry = &an_entry;
	char buf[1024];
	cptr *lines_list;

	int i, j, k, len;
	cptr tmp;

	int upper = 0, left = 0;
	int old_upper = -1, old_left = -1;
	int cx = 0, cy = 0;
	int old_cy = -1;
	int key = -1, old_key;

	bool edit_mode = FALSE;
	int dirty_line = -2;

	int wid, hgt, old_wid = -1, old_hgt = -1;

	/* Name of the Last Destroyed Item */
	last_destroyed = autopick_line_from_entry(&autopick_entry_last_destroyed);

	/* Command Description of the Last Destroyed Item */
	if (last_destroyed)
	{
		strcpy(last_destroyed_command, "^L \"");
		strncpy(last_destroyed_command + 4, last_destroyed, WID_DESC-4);
		last_destroyed_command[WID_DESC+2] = '\0';
	}
	else
	{
#ifdef JP
		strcpy(last_destroyed_command, "^L �Ǹ�˼�ư�˲����������ƥ�̾");
#else
		strcpy(last_destroyed_command, "^L Last destroyed object");
#endif
	}
	ctrl_command_desc[LAST_DESTROYED] = last_destroyed_command;

	/* Conditional Expression for Class and Race */
	sprintf(classrace, "?:[AND [EQU $CLASS %s] [EQU $RACE %s]]", 
#ifdef JP
		rp_ptr->E_title, cp_ptr->E_title
#else
		rp_ptr->title, cp_ptr->title
#endif
		);

	/* Clear yank buffer */
	yank_buf[0] = '\0';

	lines_list = read_pickpref_text_lines();

	/* Save the screen */
	screen_save();

	/* Clear screen */
	Term_clear();

	/* Process requests until done */
	while (1)
	{
		/* Get size */
		Term_get_size(&wid, &hgt);

#ifdef JP
		for (i = 0; lines_list[cy][i]; i++)
			if (iskanji(lines_list[cy][i]))
			{
				i++;
				if (i == cx)
				{
					cx--;
					break;
				}
			}
#endif

		if (cy < upper || upper + hgt - 4 <= cy)
			upper = cy - (hgt-4)/2;
		if (upper < 0)
			upper = 0;
		if ((cx < left + 10 && left > 0) || left + wid - WID_DESC - 5 <= cx)
			left = cx - (wid - WID_DESC)*2/3;
		if (left < 0)
			left = 0;

		if (old_upper != upper || old_left != left || 
		    old_wid != wid || old_hgt != hgt)
			dirty_line = -2;

		if (dirty_line == -2)
		{
			/* separator */
			for (i = 0; i < wid - WID_DESC; i++)
				buf[i] = '-';
			buf[i] = '\0';
			prt(buf, hgt - 3, 0);

			/* Display control command */
			for (i = 0; ctrl_command_desc[i]; i++)
				prt(ctrl_command_desc[i], i + 1, wid - WID_DESC);
		}

		/* Dump up to 20 lines of messages */
		for (i = 0; i < hgt - 4; i++)
		{
			int leftcol = 0;
			cptr msg;

			/* clean or dirty? */
			if (dirty_line != -2 && dirty_line != upper+i)
				continue;

			msg = lines_list[upper+i];
			if (!msg) break;

			/* Apply horizontal scroll */
			for (j = 0; *msg; msg++, j++)
			{
				if (j == left) break;
#ifdef JP
				if (j > left)
				{
					leftcol = 1;
					break;
				}
				if (iskanji(*msg))
				{
					msg++;
					j++;
				}
#endif
			}

			/* Erase line */
			Term_erase(0, i + 1, wid - WID_DESC);

			/* Dump the messages, bottom to top */
			Term_putstr(leftcol, i + 1, wid - WID_DESC - 1, TERM_WHITE, msg);
		}

		for (; i < hgt - 4; i++)
		{
			/* Erase line */
			Term_erase(0, i + 1, wid - WID_DESC);
		}

		/* Display header XXX XXX XXX */
#ifdef JP
		if (edit_mode)
			prt("^Q ESC �ǥ��ޥ�ɥ⡼�ɤذܹԡ��̾��ʸ���Ϥ��Τޤ�����", 0, 0);
		else
			prt("q �ǽ�λ��hjkl2468 �ǰ�ư��^Q a i �����ϥ⡼��", 0, 0);
#else	
		if (edit_mode)
			prt("Press ^Q ESC to command mode, any letters to insert", 0, 0);
		else
			prt("Press q to quit, hjkl2468 to move, ^Q a i to insert mode", 0, 0);
#endif
		/* Display current position */
		prt (format("(%d,%d)", cx, cy), 0, 70);

		/* Display information when updated */
		if (old_cy != cy || dirty_line == -2 || dirty_line == cy)
		{
			/* Clear information line */
			prt("", hgt - 3 + 1, 0);
			prt("", hgt - 3 + 2, 0);

			/* Display information */
			if (lines_list[cy][0] == '#')
			{
#ifdef JP
				prt("���ιԤϥ����ȤǤ���", hgt - 3 + 1, 0);
#else
				prt("This line is comment.", hgt - 3 + 1, 0);
#endif
			}
			else if (lines_list[cy][1] == ':')
			{
				switch(lines_list[cy][0])
				{
				case '?':
#ifdef JP
					prt("���ιԤϾ��ʬ�����Ǥ���", hgt - 3 + 1, 0);
#else
					prt("This line is Conditional Expression.", hgt - 3 + 1, 0);
#endif
					break;
				case 'A':
#ifdef JP
					prt("���ιԤϥޥ����μ¹����Ƥ�������ޤ���", hgt - 3 + 1, 0);
#else
					prt("This line defines Macro action.", hgt - 3 + 1, 0);
#endif
					break;
				case 'P':
#ifdef JP
					prt("���ιԤϥޥ����Υȥꥬ����������������ޤ���", hgt - 3 + 1, 0);
#else
					prt("This line defines Macro trigger key.", hgt - 3 + 1, 0);
#endif
					break;
				case 'C':
#ifdef JP
					prt("���ιԤϥ������֤�������ޤ���", hgt - 3 + 1, 0);
#else
					prt("This line defines Keymap.", hgt - 3 + 1, 0);
#endif
					break;
				}
			}
			else if (autopick_new_entry(entry, lines_list[cy]))
			{
				char temp[80*4];
				cptr t;

				describe_autopick(buf, entry);

				roff_to_buf(buf, 79, temp);
				t = temp;
				for (i = 0; i< 2; i++)
				{
					if(t[0] == 0)
						break; 
					else
					{
						prt(t, hgt - 3 + 1 + i, 0);
						t += strlen(t) + 1;
					}
				}
				autopick_free_entry(entry);
			}
		}

		/* Place cursor */
		Term_gotoxy(cx - left, cy - upper + 1);

		/* Now clean */
		dirty_line = -1;

		/* Save old key and location */
		old_cy = cy;
		old_key = key;
		old_upper = upper;
		old_left = left;
		old_wid = wid;
		old_hgt = hgt;

		/* Do not process macros except special keys */
		inkey_special = TRUE;

		/* Get a command */
		key = inkey();

		if (edit_mode)
		{
			if (key == ESCAPE)
				edit_mode = FALSE;
			else if (!iscntrl(key&0xff))
			{
				int next;

				for (i = j = 0; lines_list[cy][i] && i < cx; i++)
					buf[j++] = lines_list[cy][i];

#ifdef JP
                                if (iskanji(key))
				{
                                        inkey_base = TRUE;
                                        next = inkey();
                                        if (j+2 < 1024)
					{
                                                buf[j++] = key;
                                                buf[j++] = next;
						cx += 2;
                                        }
					else
                                                bell();
                                }
				else
#endif
				{
                                        if (j+1 < 1024)
						buf[j++] = key;
					cx++;
				}
				for (; lines_list[cy][i] && j + 1 < 1024; i++)
					buf[j++] = lines_list[cy][i];
				buf[j] = '\0';
				string_free(lines_list[cy]);
				lines_list[cy] = string_make(buf);
				len = strlen(lines_list[cy]);
				if (len < cx) cx = len;

				/* Now dirty */
				dirty_line = cy;
			}
		}
		else
		{
			/* Exit on 'q' */
			if (key == 'q') break;

			switch(key)
			{
			case 'a': case 'i':
				edit_mode = TRUE;
				break;
			case '~':
				if (!autopick_new_entry(entry, lines_list[cy]))
					break;
				string_free(lines_list[cy]);

				entry->action &= ~DO_AUTODESTROY;
				if (entry->action & DO_AUTOPICK)
				{
					entry->action &= ~DO_AUTOPICK;
					entry->action |= DONT_AUTOPICK;
				}
				else 
				{
					entry->action &= ~DONT_AUTOPICK;
					entry->action |= DO_AUTOPICK;
				}

				lines_list[cy] = autopick_line_from_entry(entry);

				/* Now dirty */
				dirty_line = cy;
				break;
			case '!':
				if (!autopick_new_entry(entry, lines_list[cy]))
					break;
				string_free(lines_list[cy]);

				entry->action &= ~DONT_AUTOPICK;
				if (entry->action & DO_AUTOPICK)
				{
					entry->action &= ~DO_AUTOPICK;
					entry->action |= DO_AUTODESTROY;
				}
				else 
				{
					entry->action &= ~DO_AUTODESTROY;
					entry->action |= DO_AUTOPICK;
				}

				lines_list[cy] = autopick_line_from_entry(entry);

				/* Now dirty */
				dirty_line = cy;
				break;
			case '(':
				key = KTRL('g');
				break;
			case '#':
			case '{':
				key = KTRL('o');
				break;
			case 'h': case '4':
				key = KTRL('b');
				break;
			case 'l': case '6':
				key = KTRL('f');
				break;
			case 'j': case '2':
				key = KTRL('n');
				break;
			case 'k': case '8':
				key = KTRL('p');
				break;
			case ' ':
				while (cy < upper + hgt-4 && lines_list[cy + 1])
					cy++;
				upper = cy;
				break;
			case '-': case 'b':
				while (0 < cy && upper <= cy)
					cy--;
				while (0 < upper && cy + 1 < upper + hgt - 4)
					upper--;
				break;
			}
		}

		switch(key)
		{
		case KTRL('a'):
			cx = 0;
			break;
		case KTRL('b'):
			if (0 < cx)
			{
				cx--;
				len = strlen(lines_list[cy]);
				if (len < cx) cx = len;
			}
			else if (cy > 0)
			{
				cy--;
				cx = strlen(lines_list[cy]);
			}
			break;
		case KTRL('c'):
			insert_return_code(lines_list, 0, cy);
			lines_list[cy] = string_make(classrace);
			cy++;
			insert_return_code(lines_list, 0, cy);
			lines_list[cy] = string_make("?:1");
			cx = 0;

			/* Now dirty */
			dirty_line = -2;
			break;
		case KTRL('e'):
			cx = strlen(lines_list[cy]);
			break;
		case KTRL('f'):
#ifdef JP
			if (iskanji(lines_list[cy][cx])) cx++;
#endif
			cx++;
			len = strlen(lines_list[cy]);
			if (len < cx)
			{
				if (lines_list[cy + 1])
				{
					cy++;
					cx = 0;
				}
				else
					cx = len;
			}
			break;
		case KTRL('g'):
			if (lines_list[cy][0] != '(' && lines_list[cy][1] != '(')
				insert_string(lines_list, "(", 0, cy);
			else if (lines_list[cy][0] == '(')
				delete_string(lines_list, 1, 0, cy);
			else if (lines_list[cy][1] == '(')
				delete_string(lines_list, 1, 1, cy);

			/* Now dirty */
			dirty_line = cy;
			break;
		case KTRL('i'):
			if (!entry_from_object(entry))
			{
				dirty_line = -2;
				break;
			}
			tmp = autopick_line_from_entry(entry);
			autopick_free_entry(entry);
			if (tmp)
			{
				insert_return_code(lines_list, 0, cy);
				lines_list[cy] = tmp;
				cx = 0;

				/* Now dirty */
				dirty_line = -2;
			}
			break;
		case KTRL('l'):
			if (last_destroyed)
			{
				insert_return_code(lines_list, 0, cy);
				lines_list[cy] = string_make(last_destroyed);
				cx = 0;

				/* Now dirty */
				dirty_line = -2;
			}
			break;
		case '\n': case '\r':
			insert_return_code(lines_list, cx, cy);
			cy++;
			cx = 0;

			/* Now dirty */
			dirty_line = -2;
			break;
		case KTRL('n'):
			if (lines_list[cy + 1]) cy++;
			break;
		case KTRL('o'):
			for (i = 0; lines_list[cy][i]; i++)
				if (lines_list[cy][i] == '#') break;
			if (!lines_list[cy][i]) insert_string(lines_list, "#", i, cy);
			cx = i + 1;
			edit_mode = TRUE;

			/* Now dirty */
			dirty_line = cy;
			break;
		case KTRL('p'):
			if (cy > 0) cy--;
			break;
		case KTRL('q'):
			edit_mode = !edit_mode;
			break;
		case KTRL('r'):
#ifdef JP
			if (!get_check("���Ƥ��ѹ����˴����Ƹ��ξ��֤��ᤷ�ޤ�����������Ǥ����� "))
#else
			if (!get_check("Discard all change and revert to original file. Are you sure? "))
#endif
				break;

			free_text_lines(lines_list);
			lines_list = read_pickpref_text_lines();
			dirty_line = -2;
			cx = cy = 0;
			edit_mode = FALSE;
			break;
		case KTRL('s'):
			if (!autopick_new_entry(entry, lines_list[cy]))
				break;
			string_free(lines_list[cy]);

			if (entry->action & DO_AUTOPICK)
			{
				entry->action &= ~DO_AUTOPICK;
				entry->action |= DO_AUTODESTROY;
			}
			else if (entry->action & DO_AUTODESTROY)
			{
				entry->action &= ~DO_AUTODESTROY;
				entry->action |= DONT_AUTOPICK;
			}
			else if (entry->action & DONT_AUTOPICK)
			{
				entry->action &= ~DONT_AUTOPICK;
				entry->action |= DO_AUTOPICK;
			}

			lines_list[cy] = autopick_line_from_entry(entry);
			/* Now dirty */
			dirty_line = cy;

			break;
		case KTRL('t'):
			break;
		case KTRL('u'):
			if (!autopick_new_entry(entry, lines_list[cy]))
				break;
			string_free(lines_list[cy]);

			if (IS_FLG(FLG_UNIDENTIFIED)) 
			{
				REM_FLG(FLG_UNIDENTIFIED);
				ADD_FLG(FLG_UNAWARE);
				REM_FLG(FLG_IDENTIFIED);
				REM_FLG(FLG_STAR_IDENTIFIED);
			}
			else if (IS_FLG(FLG_UNAWARE)) 
			{
				REM_FLG(FLG_UNIDENTIFIED);
				REM_FLG(FLG_UNAWARE);
				ADD_FLG(FLG_IDENTIFIED);
				REM_FLG(FLG_STAR_IDENTIFIED);
			}
			else if (IS_FLG(FLG_STAR_IDENTIFIED)) 
			{
				REM_FLG(FLG_UNIDENTIFIED);
				REM_FLG(FLG_UNAWARE);
				REM_FLG(FLG_IDENTIFIED);
				REM_FLG(FLG_STAR_IDENTIFIED);
			}
			else if (IS_FLG(FLG_IDENTIFIED)) 
			{
				REM_FLG(FLG_UNIDENTIFIED);
				REM_FLG(FLG_UNAWARE);
				REM_FLG(FLG_IDENTIFIED);
				ADD_FLG(FLG_STAR_IDENTIFIED);
			}
			else
			{
				ADD_FLG(FLG_UNIDENTIFIED);
				REM_FLG(FLG_UNAWARE);
				REM_FLG(FLG_IDENTIFIED);
				REM_FLG(FLG_STAR_IDENTIFIED);
			}

			lines_list[cy] = autopick_line_from_entry(entry);

			/* Now dirty */
			dirty_line = cy;
			break;
		case KTRL('v'):
			while (cy < upper + hgt-4 && lines_list[cy + 1])
				cy++;
			upper = cy;
			break;
		case KTRL('w'):
			toggle_string(lines_list, FLG_WORTHLESS, cy);
			/* Now dirty */
			dirty_line = cy;
			break;
		case KTRL('x'):
			toggle_string(lines_list, FLG_NAMELESS, cy);
			/* Now dirty */
			dirty_line = cy;
			break;
		case KTRL('y'):
			if (strlen(yank_buf))
			{
				cx = 0;
				for (j = 0; yank_buf[j]; j += strlen(yank_buf + j) + 1)
				{
					insert_return_code(lines_list, 0, cy);
					lines_list[cy] = string_make(yank_buf + j);
					cy++;
				}

				/* Now dirty */
				dirty_line = -2;
			}
			break;
		case KTRL('z'):
			toggle_string(lines_list, FLG_COLLECTING, cy);
			/* Now dirty */
			dirty_line = cy;
			break;

		case KTRL('k'):
			if (lines_list[cy][0] != '\0' && cx < strlen(lines_list[cy]))
			{
				for (i = j = 0; lines_list[cy][i] && i < cx; i++)
				{
#ifdef JP
					if (iskanji(lines_list[cy][i]))
						buf[j++] = lines_list[cy][i++];
#endif
					buf[j++] = lines_list[cy][i];
				}
				buf[j] = '\0';

				j = 0;
				if (old_key == KTRL('k'))
					while (yank_buf[j])
						j += strlen(yank_buf + j) + 1;

				if (j < MAX_YANK - 2)
				{
					strncpy(yank_buf + j, lines_list[cy] + i, MAX_YANK-j-2);
					yank_buf[MAX_YANK-2] = '\0';
					yank_buf[j + strlen(lines_list[cy] + i) + 1] = '\0';
				}
				string_free(lines_list[cy]);
				lines_list[cy] = string_make(buf);

				/* Now dirty */
				dirty_line = cy;
				break;			
			}
			/* fall through */
		case KTRL('d'):
		case 0x7F:
			/* DELETE == go forward + BACK SPACE */
#ifdef JP
			if (iskanji(lines_list[cy][cx])) cx++;
#endif
			cx++;
			len = strlen(lines_list[cy]);
			if (len < cx)
			{
				if (lines_list[cy + 1])
				{
					cy++;
					cx = 0;
				}
				else
				{
					cx = len;
					break;
				}
			}

			/* fall through */

		case '\010':
			/* BACK SPACE */
			if (cx == 0)
			{
				/* delete a return code and union two lines */
				if (cy == 0) break;
				cx = strlen(lines_list[cy-1]);
				strcpy(buf, lines_list[cy-1]);
				strcat(buf, lines_list[cy]);
				string_free(lines_list[cy-1]);
				string_free(lines_list[cy]);
				lines_list[cy-1] = string_make(buf);
				for (i = cy; lines_list[i+1]; i++)
					lines_list[i] = lines_list[i+1];
				lines_list[i] = NULL;
				cy--;

				/* Now dirty */
				dirty_line = -2;
				break;
			}

			for (i = j = 0; lines_list[cy][i] && i < cx; i++)
			{
				k = j;
#ifdef JP
				if (iskanji(lines_list[cy][i]))
					buf[j++] = lines_list[cy][i++];
#endif
				buf[j++] = lines_list[cy][i];
			}
			while (j > k)
			{
				cx--;
				j--;
			}
			for (; lines_list[cy][i]; i++)
				buf[j++] = lines_list[cy][i];
			buf[j] = '\0';
			string_free(lines_list[cy]);
			lines_list[cy] = string_make(buf);

			/* Now dirty */
			dirty_line = cy;
			break;
		}

	} /* while (1) */

	/* Restore the screen */
	screen_load();

#ifdef JP
	sprintf(buf, "picktype-%s.prf", player_name);
#else
	sprintf(buf, "pickpref-%s.prf", player_name);
#endif
	write_text_lines(buf, lines_list);
	free_text_lines(lines_list);

	/* Clear old entries */
	for( i = 0; i < max_autopick; i++)
		autopick_free_entry(&autopick_list[i]);
	max_autopick = 0;

	string_free(last_destroyed);

	/* Reload autopick pref */
	process_pickpref_file(buf);
}