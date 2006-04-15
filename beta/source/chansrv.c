#include "defines.h"
#include "vars.h"
#include "prototypes.h"

// FIXME: Double check where strtok(NULL, "") should be used instead of strok(NULL, " ")

enum chanserv_command_type
{
    INFO_COMMAND = 0,
    SAFE_COMMAND = 1,
    NORMAL_COMMAND = 2,
    DANGER_COMMAND = 3
};

enum chanserv_invoke_type
{
   DIRECT_INVOKE = 0,   // command
   ADDRESS_INVOKE = 1,  // bot: command
   MSG_INVOKE = 2,      // /msg bot command
   CHAR_INVOKE = 3      // !command
};

struct chanserv_command
{
    enum chanserv_command_type	 type;
    int                          access;
    int				 arg_count;
    struct chanserv_output 	*(*func) (char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost);
    char         		*command[5];
    char                        *syntax;
};

struct chanserv_output
{
    char                         *output;
    struct chanserv_output       *next;
};

struct chanserv_output *chanserv_show_help(char *cmd);


struct chanserv_output *chanserv_asprintf(struct chanserv_output *output, const char *format, ...)
{
	struct chanserv_output *result = NULL;
	char *ptr;
	va_list list;
	int r;

	va_start(list, format);
	r = vasprintf(&ptr, format, list);
	va_end(list);

	if (r >= 0) 
	{
	    result = malloc(sizeof(struct chanserv_output));
	    if (result)
	    {
		result->output = ptr;
		result->next = NULL;
		if (output)
		{
		    struct chanserv_output *next = output;

		    while (next->next)
			next = next->next;
		    next->next = result;
		    result = output;
		}
	    }
	}
	if (result == NULL)
	{
	    ;// FIXME: Should bitch about lack of ram.
	}
    return result;
}

void chanserv_output_free(struct chanserv_output *output)
{
    while (output)
    {
	struct chanserv_output *next = output->next;

	free(output->output);
	free(output);
	output = next;
    }
}


struct chanserv_output *chanserv_add(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;

	if (args[0] == NULL)
		return chanserv_asprintf(NULL, "Add what?");
		
	// Fix for some segmentation fault problems
	// concerning topics consisting entirely of
	// wildcard characters.
	if (strspn(args[0], "*?") == strlen(args[0]))
		return chanserv_asprintf(NULL, "Sorry, but support for that topic has been removed.");
		
	if (strlen(args[0]) > MAX_TOPIC_SIZE)
	{
		args[0][MAX_TOPIC_SIZE] = '\0';
		result = chanserv_asprintf(NULL, "Topic is over the limit, and has had characters truncated.");
	}
	s = strtok (NULL, "");
	if (s == NULL)
		return chanserv_asprintf(result, "What info is to be added for %s?", args[0]);
	if (strlen(s) > MAX_DATA_SIZE)
		s[MAX_DATA_SIZE] = '\0';
	strlwr(args[0]);
	if (*args[0] == '~')
		return chanserv_asprintf(result, "Rdb files can only be called from the data of a topic, they cannot be used in the topic itself.");
	if (check_existing_url(source, args[0], target) == 1)
		return chanserv_asprintf(result, "%s \37%s\37\n", EXISTING_ENTRY, args[0]);
#ifdef	LOG_ADD_DELETES
	db_log(ADD_DELETES, "[%s] %s!%s ADD %s %s\n", date(), source, userhost, args[0], s);
#endif
	ADDITIONS++;
	if (args[0][0] == 'i' && args[0][1] == 'l' && args[0][2] == 'c')
		db_log(URL2, "%s ([%s] %s!%s): %s\n", args[0], date(), source, userhost, s);
	else
		db_log(URL2, "%s %s\n", args[0], s);

	return chanserv_asprintf(result, "Okay.");
}

struct chanserv_output *chanserv_add_user(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char temp[1024] = { 0 };
	long sn = 0;

	if (args[0] == NULL || args[1] == NULL || args[2] == NULL || args[3] == NULL)
		return result;
	sn = atoi(args[2]);
	if (sn > 10 || sn <= 0)
		return result;
	if (strlen(args[1]) < 7)
		return result;
	sprintf(temp, "I haven't used \2%cSETINFO\2 yet!", *CMDCHAR);
	add_helper(args[0], mask_from_nick(args[1], target), sn, 0, temp, args[3], 0);
	save_changes();

	return chanserv_asprintf(NULL, "Added user: %s - level %d.", mask_from_nick(args[1], target), sn);
}

struct chanserv_output *chanserv_alarm(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	char temp[1024] = { 0 };
	long sn = 0, unixtime = 0;

	if ((args[0] == NULL) || (args[1] == NULL) || (strlen(args[0]) < 2))
		return result;
	if (*args[0] == 'd')
	{
		sn = 86400;
		args[0]++;
	}
	else if (*args[0] == 'h')
	{
		sn = 3600;
		args[0]++;
	}
	else if (*args[0] == 'm')
	{
		sn = 60;
		args[0]++;
	}
	else
		return chanserv_asprintf(NULL, "Syntax: <time type: \2d/h/m\2><time> <text to say>");
	if (strspn (args[0], NUMBER_LIST) != strlen (args[0]))
		return chanserv_asprintf(NULL, "Time must be a number.");
	snprintf(temp, sizeof (temp), "%s/%ld", DBTIMERS_PATH, (atoi (args[0]) * sn) + time (NULL));
	db_log(temp, "PRIVMSG %s :\2ALARMCLOCK\2 by %s!%s: %s", target, source, userhost, args[1]);
	unixtime = atoi (args[0]) * sn;
	if (unixtime > 86400)
		result = chanserv_asprintf(NULL, "alarmclock set to go off in %d day%s, %02d:%02d.", 
		   unixtime / 86400, (unixtime / 86400 == 1) ? "" : "s",
		   (unixtime / 3600) % 24, (unixtime / 60) % 60);
	else if (unixtime > 3600)
		result = chanserv_asprintf(NULL, "alarmclock set to go off in %d hour%s, %d min%s.",
		   unixtime / 3600, unixtime / 3600 == 1 ? "" : "s",
		   (unixtime / 60) % 60, (unixtime / 60) % 60 == 1 ? "" : "s");
	else
		result = chanserv_asprintf(NULL, "alarm clock set to go off in %d minute%s, %d sec%s.",
		   unixtime / 60, unixtime / 60 == 1 ? "" : "s", unixtime % 60,
		   unixtime % 60 == 1 ? "" : "s");

	return result;
}

struct chanserv_output *chanserv_autotopic(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		return result;
	set_autotopic (source, target, args[0]);

	return result;
}

struct chanserv_output *chanserv_backup(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	char temp[1024] = { 0 };

	snprintf(temp, sizeof (temp), "/bin/cp -rf %s \"%s.bak @ `date`\"\n", URL2, URL2);
	system (temp);

	return chanserv_asprintf(NULL, "Backed up database.");
}

struct chanserv_output *chanserv_ban_list(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	show_banlist (source);
	return result;
}

struct chanserv_output *chanserv_calc(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		return result;
	if (strlen(args[0]) > 200)
		args[0][200] = '\0';
	do_math(source, target, args[0]);

	return result;
}

struct chanserv_output *chanserv_chan_info(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		show_chaninfo (source, target, target);
	else
		/* If args[0] is not a valid channel name, just use the current channel */
		show_chaninfo (source, ((*args[0] == '#' || *args[0] == '&' || *args[0] == '+') ? args[0] : target), target);

	return result;
}

struct chanserv_output *chanserv_chan_users(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		show_chanusers (source, target);
	else
		/* If args[0] is not a valid channel name, just use the current channel. */
		show_chanusers (source, ((*args[0] == '#' || *args[0] == '&' || *args[0] == '+') ? args[0] : target));

	return result;
}

struct chanserv_output *chanserv_char(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		return result;

	return chanserv_asprintf(NULL, "%c -> %d.", args[0][0], args[0][0]);
}

struct chanserv_output *chanserv_char_show(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	return chanserv_asprintf(NULL, "My command char is: %c.", *CMDCHAR);
}

struct chanserv_output *chanserv_cpu_show(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	getrusage(RUSAGE_SELF, &r_usage);

	return chanserv_asprintf(NULL, "CPU usage: %ld.%06ld, System = %ld.%06ld.", r_usage.ru_utime.tv_sec, r_usage.ru_utime.tv_usec, r_usage.ru_stime.tv_sec, r_usage.ru_stime.tv_usec);
}

struct chanserv_output *chanserv_cycle(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;

	s = strtok (NULL, " ");
	if (s == NULL)
	{
		S("PART %s\n", target);
		S("JOIN %s\n", target);
	}
	else
	{
		S("PART %s\n", args[0]);
		S("JOIN %s\n", args[0]);
		result = chanserv_asprintf(NULL, "Cycling %s.", s);
	}

	return result;
}

struct chanserv_output *chanserv_data_search(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		return chanserv_asprintf(NULL, "What should I be %sing for?", cmd);
	datasearch (source, args[0], target);

	return result;
}

struct chanserv_output *chanserv_date(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
    return chanserv_asprintf(NULL, "%s.", date());
}

struct chanserv_output *chanserv_delban(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		return chanserv_asprintf(NULL, "Enter the user@host to purge!");
	if (del_permban(source, args[0]) == 1)
		S("MODE %s -b %s\n", target, args[0]);
	else
		result = chanserv_asprintf(NULL, "No such ban.");

	return result;
}

struct chanserv_output *chanserv_delete(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		return chanserv_asprintf(NULL, "%s what?", cmd);
	if (strlen (args[0]) > MAX_TOPIC_SIZE)
		args[0][MAX_TOPIC_SIZE] = '\0';
#ifdef	LOG_ADD_DELETES
	db_log (ADD_DELETES, "[%s] %s!%s DEL %s\n", date (), source, userhost, args[0]);
#endif
	if (*args[0] == '~')
	{	/* need level 2 to delete .rdb files */
		if (invoked == MSG_INVOKE)
		{
		    if (check_access (userhost, "#*", 0, source) >= 2)
			delete_url (source, args[0], source);
		}
		else
		{
		    if (check_access (userhost, target, 0, source) >= 2)
			delete_url (source, args[0], target);
		}
		return result;
	}
	if (invoked == MSG_INVOKE)
	    delete_url (source, args[0], source);
	else
	    delete_url (source, args[0], target);

	return result;
}

struct chanserv_output *chanserv_deluser(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		return chanserv_asprintf(NULL, "Enter the user@host to delete!");
	delete_user_ram (source, args[0]);

	return result;
}

struct chanserv_output *chanserv_deop(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;

	if (args[0] == NULL)
	    return chanserv_asprintf(NULL, "Specify a nick!");
	if (invoked == MSG_INVOKE)
	{
	    if (check_access (userhost, args[0], 0, source) >= 3)
	    {
		s = strtok (NULL, "");
		if (s == NULL)
			return result;
		S ("MODE %s -oooooo %s\n", args[0], s);
	    }
	}
	else
	{
	    S ("MODE %s -oooooo %s\n", target, args[0]);
	}

	return result;
}

struct chanserv_output *chanserv_devoice(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;

	if (args[0] == NULL)
	    return chanserv_asprintf(NULL, "Specify a nick/channel!");
	if (invoked == MSG_INVOKE)
	{
	    if (check_access (userhost, args[0], 0, source) >= 3)
	    {
		s = strtok (NULL, "");
		if (s == NULL)
			return result;
		S ("MODE %s -vvvvvv %s\n", args[0], s);
	    }
	}
	else
	{
	    S ("MODE %s -vvvvvvv %s\n", target, args[0]);
	}

	return result;
}

struct chanserv_output *chanserv_die(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;
	long unixtime = 0;

	s = strtok (NULL, "");
	if (s == NULL)
		Snow("QUIT :K\2\2illed (%s (cause I say so!))\n", source);
	else
		Snow("QUIT :K\2\2illed (%s (%s))\n", source, s);
	db_sleep (1);
#ifdef	WIN32
	printf ("\n\nGood-bye! %s (c) Jason Hamilton\n\n", dbVersion);
	uptime = time (NULL) - uptime;
	printf("Time elapsed: %ld hour%s, %ld min%s\n\n",
		 uptime / 3600,
		 uptime / 3600 == 1 ? "" : "s",
		 (uptime / 60) % 60, (uptime / 60) % 60 == 1 ? "" : "s");
	db_sleep (5);
#endif
	exit (0);

	return result;
}

struct chanserv_output *chanserv_display(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	if (args[0] == NULL)
		return result;
	display_url(target, source, args[0]);

	return result;
}

struct chanserv_output *chanserv_down(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	S ("MODE %s -o %s\n", target, source);

	return result;
}

struct chanserv_output *chanserv_darkbot(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (Sleep_Toggle == 1)
		return result;
	if (cf (userhost, source, target))
		return result;

	return chanserv_asprintf(NULL, "%s reporting! My cmdchar is %c.", dbVersion, *CMDCHAR);
}

struct chanserv_output *chanserv_google(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if(args[0] == NULL)
    	    return chanserv_asprintf(NULL, "Google what?");
	web_post_query(cmd, source, userhost, target, args[0], strlen(args[0]));

	return result;
}

struct chanserv_output *chanserv_help(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;

	s = strtok (NULL, "");
	if (s == NULL)
	{
	    result = chanserv_asprintf(result, "I can be triggered by various forms of speech, all which must be addressed to me, in one of the following formats:  %s %s %s or even %s ... In my database, you can find a topic by saying my nick, <topic> .  eg; \37%s nuke\37  ... to do a search on a word, or partial text, just type:  <mynick>, search <text> ... eg; \37%s search nuke\37.", 
		NICK_COMMA, COLON_NICK, BCOLON_NICK, Mynick, NICK_COMMA, NICK_COMMA);
	    if (cf(userhost, source, target))
		return result;
	    result = chanserv_asprintf(result, "I can also be triggered with even more human formats: \37%s who is bill gates?\37  .. You can also phrase it in a question: \37%s where is msie?\37 ...For more info about me, visit http://www.darkbot.org/ .",
		NICK_COMMA, NICK_COMMA, NICK_COMMA);
	}
	else
	    result = chanserv_show_help(s);

	return result;
}

struct chanserv_output *chanserv_idle(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	long unixtime = 0;

	if (args[0] == NULL)
		return result;
	if (stricmp (args[0], source) == 0)
		return chanserv_asprintf(NULL, "Don't be lame.");
	unixtime = return_useridle (target, args[0], 0);
	if (unixtime == 0)
		return chanserv_asprintf(NULL, "I do not see %s in %s.", args[0], target);
	unixtime = time (NULL) - unixtime;
	if (unixtime > 86400)
		result = chanserv_asprintf(result, " %s has been idle %d day%s, %02d:%02d.",
		   args[0], unixtime / 86400,
		   (unixtime / 86400 == 1) ? "" : "s",
		   (unixtime / 3600) % 24, (unixtime / 60) % 60);
	else if (unixtime > 3600)
		result = chanserv_asprintf(result, "%s has been idle %d hour%s, %d min%s.",
		   args[0], unixtime / 3600,
		   unixtime / 3600 == 1 ? "" : "s",
		   (unixtime / 60) % 60, (unixtime / 60) % 60 == 1 ? "" : "s");
	else
		result = chanserv_asprintf(result, " %s has been idle %d minute%s, %d sec%s.",
		   args[0], unixtime / 60,
		   unixtime / 60 == 1 ? "" : "s", unixtime % 60, unixtime % 60 == 1 ? "" : "s");

	return result;
}

struct chanserv_output *chanserv_ignore(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
    	    return chanserv_asprintf(NULL, "Ignore who?");
	if ( add_ignore_user_ram(args[0]) > 0 )
    	    result = chanserv_asprintf(result, "Ignoring %s.", args[0]);
	else
    	    result = chanserv_asprintf(result, "Unable to ignore %s.", args[0]);

	return result;
}

struct chanserv_output *chanserv_info(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	info (source, (invoked == MSG_INVOKE) ? source : target);
	return result;
}

struct chanserv_output *chanserv_info_2(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	show_info2 (target, source);
	return result;
}

struct chanserv_output *chanserv_info_size(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	struct stat statbuf;

	if (stat (URL2, &statbuf) == 0)
	    result = chanserv_asprintf(NULL, "My database file is presently %ld byte%s in size.", statbuf.st_size, statbuf.st_size == 1 ? "" : "s");

	return result;
}

struct chanserv_output *chanserv_isop(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		return result;
	result = chanserv_asprintf(NULL, "%s is %san op in channel %s.", args[0], is_op(args[0], target) ? "" : "not ", target);

	return result;
}

struct chanserv_output *chanserv_join(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		S ("JOIN %s\n", target);
	else
	{
		S ("JOIN %s\n", args[0]);
		result = chanserv_asprintf(NULL, "Joining %s.", args[0]);
	}

	return result;
}

struct chanserv_output *chanserv_joins_show(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	return chanserv_asprintf(NULL, "I have seen %d joins thus far.", JOINs);
}

struct chanserv_output *chanserv_jump(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;
	long sn = 0;

	if (args[0] == NULL)
		return result;
	s = strtok (NULL, " ");
	if (s == NULL)
		sn = 6667;
	else
		sn = atoi(s);
	S ("QUIT :Jumping to %s:%d\n", args[0], sn);
	db_sleep (1);
	strcpy (BS, args[0]);
	BP = sn;
	prepare_bot ();
	register_bot ();

	return result;
}

struct chanserv_output *chanserv_kick(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s2 = NULL, *s3 = NULL;

	if (args[0] == NULL)
	    return chanserv_asprintf(NULL, "Specify a nick/channel!");
	if (invoked == MSG_INVOKE)
	{
	    if (check_access (userhost, args[0], 0, source) >= 3)
	    {
		s2 = strtok (NULL, " ");
		s3 = strtok (NULL, "");
		if (s2 == NULL)
			return chanserv_asprintf(NULL, "You must specity a nick to kick from!");
		if (s3 == NULL)
			S ("KICK %s %s %s\n", args[0], s2, DEFAULT_KICK);
		else
			S ("KICK %s %s %s\n", args[0], s2, s3);
	    }
	}
	else
	{
		if (*args[0] != '#' && *args[0] != '&')
		{
			s2 = strtok (NULL, "");
			if (s2 == NULL)
			{
				if (stricmp (args[0], Mynick) == 0)
					S ("KICK %s %s :hah! As *IF*\n", target, source);
				else
					S ("KICK %s %s :\2%s\2'ed: %s\n", target, args[0], cmd, DEFAULT_KICK);
			}
			else if (stricmp (args[0], Mynick) == 0)
				S ("KICK %s %s :%s\n", target, args[0], s2);
			else
				S ("KICK %s %s :\2%s\2'ed: %s\n", target, args[0], cmd, s2);
		}
		else
		{
			s2 = strtok (NULL, " ");
			if (s2 == NULL)
				result = chanserv_asprintf(result, "You must specify a nick to kick from!");
			else
			{
				s3 = strtok (NULL, "");
				if (s3 == NULL)
				{
					if (stricmp (s2, Mynick) == 0)
						S ("KICK %s %s :hah! As *IF*\n", args[0], source);
					else
						S ("KICK %s %s :\2%s\2ed: %s\n", args[0], s2, cmd, DEFAULT_KICK);
				}
				else
				{

					if (stricmp (s2, Mynick) == 0)
						S ("KICK %s %s :hah! As *IF* (%s)\n", args[0], source);
					else
						S ("KICK %s %s :\2%s\2ed: %s\n", args[0], s2, cmd, s3);
				}
			}
		}
	}

	return result;
}

struct chanserv_output *chanserv_language(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	return chanserv_asprintf(NULL, "I speak English.");
}

struct chanserv_output *chanserv_leave(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;

	s = strtok (NULL, " ");
	if (s == NULL)
		S ("PART %s\n", target);
	else
	{
		S ("PART %s\n", s);
		result = chanserv_asprintf(result, "Leaving %s.", s);
	}

	return result;
}

struct chanserv_output *chanserv_length(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		return result;
	
	return chanserv_asprintf(NULL, "It was %d chars long.", strlen (args[0]));
}

struct chanserv_output *chanserv_level(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *uh;

	if (args[0] == NULL)
		return result;
	uh = uh_from_nick(args[0], target);
	if (uh)
	    result = chanserv_asprintf(result, "%s is level %d in channel %s.", args[0], check_access(uh, target, 0, args[0]), target);

	return result;
}

struct chanserv_output *chanserv_location_show(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	return chanserv_asprintf(NULL, "There %s %d server%s in my server list. I am currently on server #%d.", (snr == 1) ? "is" : "are", snr, (snr == 1) ? "" : "s", spr);
}

struct chanserv_output *chanserv_login(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		return result;
	do_login (source, args[0]);

	return result;
}

struct chanserv_output *chanserv_mask(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if(args[0] == NULL)
		return result;

	return chanserv_asprintf(NULL, " %s", mask_from_nick(args[0], target));
}

struct chanserv_output *chanserv_memory(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	char temp[1024] = { 0 };

	snprintf(temp, sizeof (temp), "ps -u -p %d\n", getpid());
	return chanserv_asprintf(NULL, "ps: %s", run_program(temp));
}

struct chanserv_output *chanserv_metar(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

        if(args[0] == NULL)
            return chanserv_asprintf(NULL, "Metar what?");
        web_post_query (cmd, source, userhost, target, args[0], strlen(args[0]));

	return result;
}

struct chanserv_output *chanserv_nick(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	// If no nick was specified...
	if(args[0] == NULL)
		return chanserv_asprintf(NULL, "Specify a nick!");
	
	// If the nick specified contains illegal characters...
	if(strspn(args[0], LEGAL_NICK_TEXT) != strlen(args[0]))
		return chanserv_asprintf(NULL, "The nickname %s contains illegal characters.", args[0]);

	strncpy(Mynick, args[0], sizeof (Mynick));
	strncpy(s_Mynick, Mynick, sizeof (s_Mynick));
	snprintf(NICK_COMMA, sizeof (NICK_COMMA), "%s,", Mynick);
	snprintf(COLON_NICK, sizeof (COLON_NICK), "%s:", Mynick);
	snprintf(BCOLON_NICK, sizeof (BCOLON_NICK), "%s\2:\2", Mynick);
	// FIXME: This should be sent before the NICK attempt, and/or complain if the NICK doesn't work.
	result = chanserv_asprintf(result, "Attempting to /nick %s.", Mynick);
	S("NICK %s\n", Mynick);

	return result;
}

struct chanserv_output *chanserv_op(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;

	if (args[0] == NULL)
	    return chanserv_asprintf(NULL, "Specify a nick!");
	if (invoked == MSG_INVOKE)
	{
	    if (check_access (userhost, args[0], 0, source) >= 3)
	    {
		s = strtok (NULL, "");
		if (s == NULL)
			return result;
		S ("MODE %s +oooooo %s\n", args[0], s);
	    }
	}
	else
	{
	    S ("MODE %s +oooooo %s\n", target, args[0]);
	}

	return result;
}

struct chanserv_output *chanserv_os_show(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	char temp[1024] = { 0 };

#ifdef	WIN32
	snprintf (temp, sizeof (temp), "cmd /c ver\n");
#else				
	snprintf (temp, sizeof (temp), "uname\n");
#endif

	return chanserv_asprintf(NULL, "I am running %s.", run_program(temp));	
}

struct chanserv_output *chanserv_password(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL || args[1] == NULL)
		return result;
	if (strlen(args[1]) > 25)
		args[1][25] = '\0';
	set_pass(source, userhost, args[0], args[1]);

	return result;
}

struct chanserv_output *chanserv_performs(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	/* Set the default umodes */
	S ("MODE %s %s\n", Mynick, DEFAULT_UMODE);
	run_perform ();

	return chanserv_asprintf(NULL, "Performs have been executed.");
}

struct chanserv_output *chanserv_perm_ban(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;

	if (args[0] == NULL)
		return result;
	s = strtok (NULL, "");
	if (s == NULL)
		s = "Permbanned!";
	add_permban(args[0], 0, s);
	result = chanserv_asprintf(result, "Added in permban #%d, %s; reason: %s.", PERMBAN_counter, args[0], s);
	save_permbans();
	S("MODE %s +b %s\n", target, args[0]);

	return result;
}

struct chanserv_output *chanserv_perm_bans_list(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	return chanserv_asprintf(NULL, "There %s %d permban%s loaded into ram.", (PERMBAN_counter == 1) ? "is" : "are", PERMBAN_counter, (PERMBAN_counter == 1) ? "" : "s");
}

struct chanserv_output *chanserv_ping(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (cf(userhost, source, target))
		return result;
	if (cf(userhost, source, target))
		return result;
	if (args[0] != NULL)
	{
		if (strlen (args[0]) > 21)
			args[0][21] = '\0';
		S ("NOTICE %s :\1PING %s\n", source, args[0]);
	}

	return result;
}

struct chanserv_output *chanserv_ping2(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (check_access (userhost, target, 0, source) == 0)
		S ("NOTICE %s PONG!\n", source);
	else
		S ("PRIVMSG %s :PONG!\n", target);

	return result;
}

struct chanserv_output *chanserv_queue_show(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	return chanserv_asprintf(NULL, "There is currently %d item%s in queue.", get_sendq_count(2), (get_sendq_count(2) == 1) ? "" : "s");
}

struct chanserv_output *chanserv_quiz(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (quiz_halt == 0)
		run_quiz_question (target);

	return result;
}

struct chanserv_output *chanserv_quote(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	do_randq(args[0], RANDQ_RAND, target, source);

	return result;
}

struct chanserv_output *chanserv_random_quote(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	// RANDQ_NORMAL
	do_randq(args[0], RANDQ_NORMAL, target, source);

	return result;
}

struct chanserv_output *chanserv_random_quote_2(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	// RANDQ_CASE
	do_randq(args[0], RANDQ_CASE, target, source);

	return result;
}

struct chanserv_output *chanserv_random_stuff(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;

	if (args[0] == NULL)
	    return chanserv_asprintf(NULL, "What do you want to add?");
	if (invoked == MSG_INVOKE)
	{
	    if (check_access (userhost, args[0], 0, source) >= RAND_LEVEL)
	    {
		s = strtok (NULL, "");
		if (s == NULL)
			return chanserv_asprintf(NULL, "What do you want to add?");
		add_randomstuff(source, source, s);
	    }
	}
	else
	    add_randomstuff(source, target, args[0]);

	return result;
}

struct chanserv_output *chanserv_random_stuff_list(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	return chanserv_asprintf(NULL, "%d seconds left till randstuff.", Rand_Stuff);
}

struct chanserv_output *chanserv_raw(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;

	s = strtok (NULL, "");
	if (s != NULL)
		S("%s\n", s);

	return result;
}

struct chanserv_output *chanserv_rdb(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char temp[1024] = { 0 };
	char *s = NULL;

	s = strtok (NULL, " ");
	if (s == NULL)
	{
		snprintf(temp, sizeof (temp), "ls %s/*.rdb | wc\n", RDB_DIR);
		result = chanserv_asprintf(result, "RDB: %s.", run_program(temp));
	}
	else
	{
		if (strspn(s, SAFE_LIST) != strlen(s))
			return chanserv_asprintf(NULL, "Rdb files are made up of letters and or numbers, no other text is accepted.");
		snprintf(temp, sizeof (temp), "cat %s/%s.rdb | wc -l\n", RDB_DIR, s);
		result = chanserv_asprintf(result, ":%s", run_program(temp));
	}

	return result;
}

struct chanserv_output *chanserv_repeat(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	long sn2 = 0, sn = 0;

	if (args[0] == NULL || args[1] == NULL || args[2] == NULL)
		return result;
	sn = atoi (args[0]);
	sn2 = atoi (args[1]);
	while (sn > 0)
	{
		S ("%s\n", args[2]);
		sn--;
		db_sleep (sn2);
	}

	return result;
}

struct chanserv_output *chanserv_replace(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;

	if (args[0] == NULL)
		return chanserv_asprintf(NULL, "Replace what?");
	if (strlen(args[0]) > MAX_TOPIC_SIZE)
		args[0][MAX_TOPIC_SIZE] = '\0';
	s = strtok (NULL, "");
	if (s == NULL)
		return chanserv_asprintf(NULL, "What info should replace %s?", args[0]);
	if (strlen(s) > MAX_DATA_SIZE)
		s[MAX_DATA_SIZE] = '\0';
	strlwr(args[0]);
	if (check_existing_url(source, args[0], target) != 1)
		return chanserv_asprintf(NULL, "%s \37%s\37", NO_ENTRY, args[0]);
	delete_url (source, args[0], target);
#ifdef	LOG_ADD_DELETES
	db_log (ADD_DELETES, "[%s] %s!%s REPLACE %s %s\n", date (), source, userhost, args[0], s);
#endif
	ADDITIONS++;
	db_log (URL2, "%s %s\n", args[0], s);

	return chanserv_asprintf(NULL, "%s has been updated.", args[0]);
}

struct chanserv_output *chanserv_reserved_1(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

        if (args[0] == NULL)
            return result;
        call_reserved_1(source, target, args[0]);

	return result;
}

struct chanserv_output *chanserv_reserved_2(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

        if (args[0] == NULL)
            return result;
        call_reserved_2(source, target, args[0]);

	return result;
}

struct chanserv_output *chanserv_restart(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char temp[1024] = { 0 };

	S("QUIT :Restarting %s ...\n", dbVersion);
	db_sleep(2);
	snprintf(temp, sizeof (temp), "%s", DARKBOT_BIN);
	system(temp);
	db_sleep(1);
	exit(0);

	return result;
}

struct chanserv_output *chanserv_search(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
	{
		if (stricmp(cmd, "FIND") == 0)
			result = chanserv_asprintf(NULL, "%s?", TRY_FIND);
		else
			result = chanserv_asprintf(NULL, "What should I be %sing for?", cmd);
		return result;
	}
	find_url(source, args[0], target);

	return result;
}

struct chanserv_output *chanserv_seen(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
	{
		count_seen(source, target);
		return result;
	}
	if (return_useridle (target, args[0], 1) == 1)
		return chanserv_asprintf(NULL, "%s is right here in the channel!", args[0]);
	show_seen(args[0], source, target);

	return result;
}

struct chanserv_output *chanserv_setchan(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		return result;
	strncpy (CHAN, args[0], sizeof (CHAN));
	result = chanserv_asprintf(result, "Default channel: %s.", CHAN);
	save_setup ();

	return result;
}

struct chanserv_output *chanserv_setchar(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		return result;
	*CMDCHAR = *args[0];
	result = chanserv_asprintf(result, "New command char now: %c.", *CMDCHAR);
	save_setup ();

	return result;
}

struct chanserv_output *chanserv_setinfo(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		return chanserv_asprintf(NULL, "My !setinfo variables are: ^ nick, % Number of joins, & Channel, $ user@host. Example: !setinfo ^ has joined & % times!!  (also, if you make the first char of your SETINFO a \"+\" the setinfo will be shown as an ACTION).");
	update_setinfo (userhost, args[0], source);
	save_changes ();

	return result;
}

struct chanserv_output *chanserv_setnick(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		return result;

	if (strspn (args[0], LEGAL_NICK_TEXT) != strlen (args[0]))
		return chanserv_asprintf(NULL, "The nickname %s contains illegal characters.", args[0]);

	S("NICK %s\n", args[0]);
	strncpy(s_Mynick, args[0], sizeof (s_Mynick));
	strncpy(Mynick, args[0], sizeof (Mynick));
	snprintf(NICK_COMMA, sizeof (NICK_COMMA), "%s,", Mynick);
	snprintf(COLON_NICK, sizeof (COLON_NICK), "%s:", Mynick);
	snprintf(BCOLON_NICK, sizeof (BCOLON_NICK), "%s\2:\2", Mynick);
	save_setup();

	return result;
}

struct chanserv_output *chanserv_setuser(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		return result;
	strncpy (UID, args[0], sizeof (UID));
	save_setup ();

	return chanserv_asprintf(NULL, "Default userid now: %s.", UID);
}

struct chanserv_output *chanserv_sleep(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;

	Sleep_Toggle = 1;
					
	if ((s = strtok (NULL, "")) == NULL)
		Sleep_Time = SLEEP_TIME;
	else if ((Sleep_Time = strtol (s, (char **) NULL, Sleep_Time)) < 1)
		Sleep_Time = SLEEP_TIME;

	S ("PRIVMSG %s :%s\n", target, GOSLEEP_ACTION);
	strncpy (sleep_chan, target, sizeof (sleep_chan));

	/* If the user has specified a custom length of time to sleep for, send
	 * a notice reminding the user how long the bot will be asleep, in a 
	 * more readible format.
	 */
	if (Sleep_Time != SLEEP_TIME)
	{
		if (Sleep_Time > 86400)
			result = chanserv_asprintf(result, "Sleeping for %ld day%s, %02ld:%02ld.",
				Sleep_Time / 86400, 
				(Sleep_Time / 86400 == 1) ? "" : "s",
				(Sleep_Time / 3600) % 24, 
				(Sleep_Time / 60) % 60);
		else if (Sleep_Time > 3600)
			result = chanserv_asprintf(result, "Sleeping for %ld hour%s, %ld min%s.",
				Sleep_Time / 3600, 
				Sleep_Time / 3600 == 1 ? "" : "s",
				(Sleep_Time / 60) % 60, 
				(Sleep_Time/ 60) % 60 == 1 ? "" : "s");
		else
			result = chanserv_asprintf(result, "Sleeping for %ld minute%s, %ld sec%s.",
				Sleep_Time / 60,
				Sleep_Time / 60 == 1 ? "" : "s",
				Sleep_Time % 60,
				Sleep_Time % 60 == 1 ? "" : "s");
	}

	return result;
}

struct chanserv_output *chanserv_stats(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	get_stats(target, strtok (NULL, " "));

	return result;
}

struct chanserv_output *chanserv_taf(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

        if(args[0] == NULL)
            return chanserv_asprintf(NULL, "Taf what?");
        web_post_query(cmd, source, userhost, target, args[0], strlen(args[0]));

	return result;
}

struct chanserv_output *chanserv_teaseop(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
		return chanserv_asprintf(NULL, "Specify a nick!");
	if (stricmp (args[0], Mynick) == 0)
		result = chanserv_asprintf(result, "How about I not do that?");
	else
		S ("MODE %s +o-o+o-o+o-o %s %s %s %s %s %s\n", target, args[0], args[0], args[0], args[0], args[0], args[0]);

	return result;
}

struct chanserv_output *chanserv_tell(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;

	if (args[0] == NULL)
	    return chanserv_asprintf(NULL, "Tell who?");
	if (args[1] == NULL)
	    return chanserv_asprintf(NULL, "What do you want me to tell %s?", args[0]);
	if (stricmp (args[1], Mynick) == 0)
	    return result;			/* don't bother telling myself about stuff */
	if (stricmp (args[1], "ABOUT") == 0)
	{
		s = strtok (NULL, " ");
		if (s == NULL)
		    return chanserv_asprintf(NULL, "Tell %s about what?", args[0]);
		strlwr(s);
		if (invoked == MSG_INVOKE)
		    show_url (source, get_multiword_topic (s), args[0], 1, 0, userhost, 1);
		else
		    show_url (args[0], get_multiword_topic (s), target, 1, 0, userhost, 1);
	}
	else
	{
		strlwr(args[1]);
		if (invoked == MSG_INVOKE)
		    show_url (source, get_multiword_topic (args[1]), args[0], 1, 0, userhost, 1);
		else
		    show_url (args[0], get_multiword_topic (args[1]), target, 1, 0, userhost, 1);
	}

	return result;
}

struct chanserv_output *chanserv_topic(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;

	s = strtok (NULL, "");
	if (s == NULL)
		S ("TOPIC %s :\n", target);
	else
		S ("TOPIC %s :%s\n", target, s);

	return result;
}

struct chanserv_output *chanserv_unignore(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
    	    return chanserv_asprintf(NULL, "Unignore who?");
	if ( delete_ignore_user_ram (args[0]) > 0 )
    	    result = chanserv_asprintf(result, "Unignoring %s.", args[0]);
	else
    	    result = chanserv_asprintf(result, "Unable to unignore %s. :(", args[0]);

	return result;
}

struct chanserv_output *chanserv_unixtime(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	long unixtime = 0;

	if (args[0] == NULL)
		return result;
	unixtime = atoi (args[0]) - time (NULL);
	if (unixtime > 86400)
		result = chanserv_asprintf(result, "%d day%s, %02d:%02d.",
		   unixtime / 86400,
		   (unixtime / 86400 == 1) ? "" : "s",
		   (unixtime / 3600) % 24, (unixtime / 60) % 60);
	else if (unixtime > 3600)
		result = chanserv_asprintf(result, "%d hour%s, %d min%s.",
		   unixtime / 3600,
		   unixtime / 3600 == 1 ? "" : "s",
		   (unixtime / 60) % 60, (unixtime / 60) % 60 == 1 ? "" : "s");
	else
		result = chanserv_asprintf(result, "%d minute%s, %d sec%s.",
		   unixtime / 60,
		   unixtime / 60 == 1 ? "" : "s", unixtime % 60, unixtime % 60 == 1 ? "" : "s");

	return result;
}

struct chanserv_output *chanserv_up(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	S ("MODE %s +o %s\n", target, source);
	return result;
}

struct chanserv_output *chanserv_uptime(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	char temp[1024] = { 0 };

	snprintf(temp, sizeof (temp), "uptime\n");
	return chanserv_asprintf(NULL, "Uptime: %s.", run_program(temp));
}

struct chanserv_output *chanserv_user_list(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;

	if ((s = strtok (NULL, " ")) != NULL)
		show_helper_list (source, atoi (s));
	else
		show_helper_list (source, 0);

	return result;
}

struct chanserv_output *chanserv_users_list(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	S ("LUSERS\n");

	return result;
}

struct chanserv_output *chanserv_variables(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	return chanserv_asprintf(NULL, "Data variables are: N~ (Nick), C~ (Chan), T~ (Time/date) B~ (Botnick), Q~ (Question asked), R~ (random nick), !~ (command char), S~ (current Server), P~ (current port) V~ (botVer), W~ (db WWW site), H~ (u@h), t~ (unixtime), BAN (sets a ban), TEMPBAN (bans for 60 sec).");
}

struct chanserv_output *chanserv_version(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (cf (userhost, source, target))
		return result;
	if (cf (userhost, source, target))
		return result;

	return chanserv_asprintf(NULL, "\1VERSION Hi, I'm a Darkbot. Download me from http://www.darkbot.org\1.");
}

struct chanserv_output *chanserv_vhost(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;

	if (args[0] == NULL)
	    return result;
	strncpy(VHOST, args[0], sizeof (VHOST));
	save_setup();

	return chanserv_asprintf(NULL, "NOTICE %s :Default Vhost now: %s.", VHOST);
}

struct chanserv_output *chanserv_voice(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;

	if (args[0] == NULL)
	    return chanserv_asprintf(NULL, "Specify a nick/channel!");
	if (invoked == MSG_INVOKE)
	{
	    if (check_access (userhost, args[0], 0, source) >= 3)
	    {
		s = strtok (NULL, "");
		if (s == NULL)
			return result;
		S ("MODE %s +vvvvvv %s\n", args[0], s);
	    }
	}
	else
	    S ("MODE %s +vvvvvvv %s\n", target, args[0]);

	return result;
}

struct chanserv_output *chanserv_wakeup(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	if (Sleep_Toggle == 0)
		return result;
	Sleep_Toggle = 0;
	AIL4 = 0;
	S ("PRIVMSG %s :%s\n", target, WAKEUP_ACTION);
	if (stricmp (sleep_chan, target) != 0)
		S ("PRIVMSG %s :%s\n", sleep_chan, WAKEUP_ACTION);

	return result;
}

struct chanserv_output *chanserv_weather(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	if (args[0] == NULL)
		return chanserv_asprintf(NULL, "Show weather from where?");
	web_post_query(cmd, source, userhost, target, args[0], strlen(args[0]));

	return result;
}

struct chanserv_output *chanserv_where(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;
	char *ptr3 = NULL;

	if (args[0] == NULL)
	    return chanserv_asprintf(NULL, "You were asking?");
	if (args[1] == NULL)
		return result;
	strlwr (args[1]);
	ptr3 = strchr (args[1], '?');
	if (ptr3 != NULL)
		memmove (ptr3, ptr3 + 1, strlen (ptr3 + 1) + 1);
	ptr3 = strchr (args[1], '!');
	if (ptr3 != NULL)
		memmove (ptr3, ptr3 + 1, strlen (ptr3 + 1) + 1);
	if (stricmp (args[1], "A") == 0 || stricmp (args[1], "AN") == 0)
	{
		s = strtok (NULL, " ");
		if (s == NULL)
		    return chanserv_asprintf(NULL, "%s %s %s? Mind rephrasing that?  (Type %cHELP for syntax hints).", cmd, args[0], args[1], *CMDCHAR);
		show_url (source, get_multiword_topic (s), (invoked == MSG_INVOKE) ? source : target, 1, 0, userhost, 0);
	}
	else
		show_url (source, get_multiword_topic (args[1]), (invoked == MSG_INVOKE) ? source : target, 1, 0, userhost, 0);

	return result;
}

struct chanserv_output *chanserv_whisper(char *source, char *target, char *cmd, char **args, enum chanserv_invoke_type invoked, char *userhost)
{
	struct chanserv_output *result = NULL;
	char *s = NULL;

	if (args[0] == NULL)
		return chanserv_asprintf(NULL, "Whisper to who?");
	if (args[1] == NULL)
		return chanserv_asprintf(NULL, "What do you want me to whisper to %s?", args[0]);
	if (stricmp (args[1], Mynick) == 0)
		return result;		/* don't bother telling myself about stuff */
	if (stricmp (args[1], "ABOUT") == 0)
	{
		s = strtok (NULL, " ");
		if (s == NULL)
			return chanserv_asprintf(NULL, "Whisper to %s about what?", args[0]);
		strlwr(s);
		show_url(source, get_multiword_topic(s), args[0], 1, 0, userhost, 1);
	}
	else
	{
		strlwr (args[1]);
		show_url(source, get_multiword_topic(args[1]), args[0], 1, 0, userhost, 1);
	}

	return result;
}

struct chanserv_command chanserv_commands[] =
{
    {DANGER_COMMAND,  1, 1, chanserv_add,		{"ADD", "REMEMBER", "SAVE", "STORE", NULL}, "<topic> <text>"},
    {DANGER_COMMAND,  3, 4, chanserv_add_user,		{"ADDUSER", NULL, NULL, NULL, NULL}, "<#channel|#*> <user@host> <level> [password]"},
    {SAFE_COMMAND,    2, 2, chanserv_alarm,		{"ALARM", "ALARMCLOCK", NULL, NULL, NULL}, "<time type: d/h/m><time> <text to say>"},
    {DANGER_COMMAND,  3, 1, chanserv_autotopic,		{"AUTOTOPIC", NULL, NULL, NULL, NULL}, "<channel topic>  (set to \"0\" to turn off)"},
#ifndef	WIN32
    {DANGER_COMMAND,  3, 0, chanserv_backup,		{"BACKUP", NULL, NULL, NULL, NULL}, NULL},
#endif
    {INFO_COMMAND,    1, 0, chanserv_ban_list,		{"BANLIST", NULL, NULL, NULL, NULL}, NULL},
#ifdef	DO_MATH_STUFF
    {INFO_COMMAND,   -1, 1, chanserv_calc,		{"CALC", "MATH", NULL, NULL, NULL}, "<expression>"},
#endif
    {INFO_COMMAND,   -1, 1, chanserv_chan_info,		{"CHANINFO", NULL, NULL, NULL, NULL}, "<#channel>"},
    {INFO_COMMAND,   -1, 1, chanserv_chan_users,	{"CHANUSERS", NULL, NULL, NULL, NULL}, "<#channel>"},
    {INFO_COMMAND,   -1, 1, chanserv_char,		{"CHAR", NULL, NULL, NULL, NULL}, "<character>"},
    {INFO_COMMAND,   -1, 0, chanserv_char_show,		{"CMDCHAR?", NULL, NULL, NULL, NULL}, NULL},
    {INFO_COMMAND,   -1, 0, chanserv_cpu_show,		{"CPU?", NULL, NULL, NULL, NULL}, NULL},
    {DANGER_COMMAND,  2, 0, chanserv_cycle,		{"CYC", "CYCLE", NULL, NULL, NULL}, "[#channel]"},
    {INFO_COMMAND,   -1, 1, chanserv_data_search,	{"DATASEARCH", "DSEARCH", "DFIND", NULL, NULL}, "<topic>"},
    {INFO_COMMAND,   -1, 0, chanserv_date,		{"DATE", "TIME", NULL, NULL, NULL}, NULL},
#if DO_CHANBOT_CRAP == 1
    {DANGER_COMMAND,  3, 1, chanserv_delban,		{"DELBAN", NULL, NULL, NULL, NULL}, "<user@host>"},
#endif
    {DANGER_COMMAND,  1, 1, chanserv_delete,		{"DELETE", "DEL", "REMOVE", "FORGET", NULL}, "<topic>"},
    {DANGER_COMMAND,  3, 1, chanserv_deluser,		{"DELUSER", NULL, NULL, NULL, NULL}, "<user@host>"},
#if DO_CHANBOT_CRAP == 1
    {DANGER_COMMAND,  3, 1, chanserv_deop,		{"DEOP", NULL, NULL, NULL, NULL}, "<nick>"},
    {DANGER_COMMAND,  3, 1, chanserv_devoice,		{"DEV", "DV", "DEVOICE", "DVOICE", NULL}, "<nick>"},
#endif
    {DANGER_COMMAND,  3, 0, chanserv_die,		{"DIE", "QUIT", NULL, NULL, NULL}, NULL},
    {INFO_COMMAND,   -1, 1, chanserv_display,		{"DISPLAY", NULL, NULL, NULL, NULL}, "<topic>"},
#if DO_CHANBOT_CRAP == 1
    {DANGER_COMMAND,  2, 0, chanserv_down,		{"DOWN", NULL, NULL, NULL, NULL}, NULL},
#endif
    {INFO_COMMAND,   -1, 0, chanserv_darkbot,		{"\2\2DARKBOT", NULL, NULL, NULL, NULL}, NULL},
#if GOOGLE == 1
    {NORMAL_COMMAND, -1, 1, chanserv_google,		{"GOOGLE", NULL, NULL, NULL, NULL}, "<text>"},
#endif
    {INFO_COMMAND,   -1, 0, chanserv_help,		{"HELP", NULL, NULL, NULL, NULL}, "[command]"},
    {INFO_COMMAND,   -1, 1, chanserv_idle,		{"IDLE", NULL, NULL, NULL, NULL}, "<nick>"},
    {DANGER_COMMAND,  1, 1, chanserv_ignore,		{"IGNORE", NULL, NULL, NULL, NULL}, "<nick>"},
    {INFO_COMMAND,   -1, 0, chanserv_info,		{"INFO", NULL, NULL, NULL, NULL}, NULL},
    {INFO_COMMAND,   -1, 0, chanserv_info_2,		{"INFO2", NULL, NULL, NULL, NULL}, NULL},
    {INFO_COMMAND,   -1, 1, chanserv_isop,		{"ISOP", NULL, NULL, NULL, NULL}, "<nick>"},
    {INFO_COMMAND,    2, 0, chanserv_info_size,		{"DBSIZE", "INFOSIZE", NULL, NULL, NULL}, NULL},
    {DANGER_COMMAND,  2, 1, chanserv_join,		{"JOIN", "J", NULL, NULL, NULL}, "<#channel>"},
    {INFO_COMMAND,   -1, 0, chanserv_joins_show,	{"JOINS?", NULL, NULL, NULL, NULL}, NULL},
    {DANGER_COMMAND,  3, 1, chanserv_jump,		{"JUMP", "SERVER", NULL, NULL, NULL}, "<server> [port]"},
#if DO_CHANBOT_CRAP == 1
    {DANGER_COMMAND,  3, 1, chanserv_kick,		{"KICK", "WHACK", "K", "NAIL", NULL}, "[#channel] <nick> [reason]"},
#endif
    {INFO_COMMAND,   -1, 0, chanserv_language,		{"LANG", "LANGUAGE", NULL, NULL, NULL}, NULL},
    {DANGER_COMMAND,  2, 0, chanserv_leave,		{"L", "PART", "LEAVE", "P", NULL}, "[#channel]"},
    {INFO_COMMAND,   -1, 1, chanserv_length,		{"LENGTH", NULL, NULL, NULL, NULL}, "<text>"},
    {INFO_COMMAND,   -1, 1, chanserv_level,		{"LEVEL", NULL, NULL, NULL, NULL}, "<nick>"},
    {INFO_COMMAND,   -1, 0, chanserv_location_show,	{"LOCATION?", NULL, NULL, NULL, NULL}, NULL},
    {NORMAL_COMMAND, -1, 1, chanserv_login,		{"LOGIN", NULL, NULL, NULL, NULL}, "<password>"},
    {INFO_COMMAND,   -1, 1, chanserv_mask,		{"MASK", NULL, NULL, NULL, NULL}, "<nick>"},
#ifndef	WIN32
    {INFO_COMMAND,    3, 0, chanserv_memory,		{"MEM", "RAM", NULL, NULL, NULL}, NULL},
#endif
#if METAR == 1
    {NORMAL_COMMAND, -1, 1, chanserv_metar,		{"METAR", NULL, NULL, NULL, NULL}, "<city or code>"},
#endif
    {DANGER_COMMAND,  3, 1, chanserv_nick,		{"N", "NICK", NULL, NULL, NULL}, "<newnick>"},
#if DO_CHANBOT_CRAP == 1
    {DANGER_COMMAND,  3, 1, chanserv_op,		{"OP", NULL, NULL, NULL, NULL}, "[#channel] <nick>"},
#endif
    {INFO_COMMAND,   -1, 0, chanserv_os_show,		{"OS", NULL, NULL, NULL, NULL}, NULL},
    {DANGER_COMMAND, -1, 2, chanserv_password,		{"PASS", "PASSWORD", "PASSWD", NULL, NULL}, "<old password> <new password>"},
    {DANGER_COMMAND,  3, 0, chanserv_performs,		{"PERFORMS", NULL, NULL, NULL, NULL}, NULL},
#if DO_CHANBOT_CRAP == 1
    {DANGER_COMMAND,  3, 1, chanserv_perm_ban,		{"PERMBAN", "SHITLIST", NULL, NULL, NULL}, "<user@host> [reason]"},
    {INFO_COMMAND,   -1, 0, chanserv_perm_bans_list,	{"PERMBANS?", NULL, NULL, NULL, NULL}, NULL},
#endif
#if CTCP == 1
    {INFO_COMMAND,   -1, 1, chanserv_ping,		{"\1PING", NULL, NULL, NULL, NULL}, "<>"},
#endif
    {INFO_COMMAND,   -1, 0, chanserv_ping2,		{"PING", NULL, NULL, NULL, NULL}, NULL},
    {INFO_COMMAND,   -1, 0, chanserv_queue_show,	{"SENDQ?", "QUE?", NULL, NULL, NULL}, NULL},
#if QUIZ == 1
    {SAFE_COMMAND,   -1, 0, chanserv_quiz,		{"QUIZ", NULL, NULL, NULL, NULL}, NULL},
#endif
#if RANDQ == ON
    {NORMAL_COMMAND, -1, 1, chanserv_quote,		{"QUOTE", NULL, NULL, NULL, NULL}, "<>"},
    {NORMAL_COMMAND, -1, 1, chanserv_random_quote,	{"RANDQ", "RANDQUOTE", NULL, NULL, NULL}, "<>"},
    {NORMAL_COMMAND, -1, 1, chanserv_random_quote_2,	{"RANDQ2", "RANDQUOTE2", NULL, NULL, NULL}, "<>"},
#endif
#ifdef	RANDOM_STUFF
    {DANGER_COMMAND, RAND_LEVEL, 1, chanserv_random_stuff,	{"RANDOMSTUFF", "RANDSTUFF", "RS", NULL, NULL}, "<text>"},
    {INFO_COMMAND,   -1, 0, chanserv_random_stuff_list,	{"RANDSTUFF?", "RANDOMSTUFF?", NULL, NULL, NULL}, NULL},
#endif
    {DANGER_COMMAND,  3, 1, chanserv_raw,		{"RAW", NULL, NULL, NULL, NULL}, "<raw data>"},
#ifndef	WIN32
    {INFO_COMMAND,   -1, 0, chanserv_rdb,		{"RDB", NULL, NULL, NULL, NULL}, "[topic]"},
#endif
    {DANGER_COMMAND,  3, 3, chanserv_repeat,		{"REPEAT", "TIMER", NULL, NULL, NULL}, "<number> <delay> <raw data>"},
    {DANGER_COMMAND,  1, 1, chanserv_replace,		{"REPLACE", NULL, NULL, NULL, NULL}, "<topic> <text>"},
    {DANGER_COMMAND, -1, 1, chanserv_reserved_1,	{RESERVED1, NULL, NULL, NULL, NULL}, "<>"},
    {DANGER_COMMAND, -1, 1, chanserv_reserved_2,	{RESERVED2, NULL, NULL, NULL, NULL}, "<>"},
    {DANGER_COMMAND,  3, 0, chanserv_restart,		{"REHASH", "RESTART", NULL, NULL, NULL}, NULL},
    {INFO_COMMAND,   -1, 1, chanserv_search,		{"SEARCH", "LOOK", "FIND", NULL, NULL}, "<text>"},
    {INFO_COMMAND,   -1, 1, chanserv_seen,		{"SEEN", NULL, NULL, NULL, NULL}, "<nick>"},
    {DANGER_COMMAND,  3, 1, chanserv_setchan,		{"SETCHAN", NULL, NULL, NULL, NULL}, "<new #channel>"},
    {DANGER_COMMAND,  3, 1, chanserv_setchar,		{"SETCHAR", NULL, NULL, NULL, NULL}, "<new command char>"},
    {DANGER_COMMAND,  1, 1, chanserv_setinfo,		{"SETINFO", NULL, NULL, NULL, NULL}, "<new user greeting|0>"},
    {DANGER_COMMAND,  3, 1, chanserv_setnick,		{"SETNICK", NULL, NULL, NULL, NULL}, "<new nick>"},
    {DANGER_COMMAND,  3, 1, chanserv_setuser,		{"SETUSER", NULL, NULL, NULL, NULL}, "<new userid> (requires a restart)"},
    {SAFE_COMMAND, SLEEP_LEVEL, 0, chanserv_sleep,	{"SLEEP", "HUSH", NULL, NULL, NULL}, NULL},
    {INFO_COMMAND,   -1, 0, chanserv_stats,		{"STATS", NULL, NULL, NULL, NULL}, "[nick]"},
#if TAF == 1
    {NORMAL_COMMAND, -1, 1, chanserv_taf,		{"TAF", NULL, NULL, NULL, NULL}, "<city or code>"},
#endif
#if DO_CHANBOT_CRAP == 1
    {DANGER_COMMAND,  2, 1, chanserv_teaseop,		{"TEASEOP", "TO", NULL, NULL, NULL}, "<nick>"},
#endif
    {INFO_COMMAND, -1, 2, chanserv_tell,		{"TELL", NULL, NULL, NULL, NULL}, "<nick> [ABOUT] <topic>"},
#if DO_CHANBOT_CRAP == 1
    {DANGER_COMMAND,  2, 0, chanserv_topic,		{"T", "TOPIC", NULL, NULL, NULL}, "<channel topic>"},
#endif
    {DANGER_COMMAND,  1, 1, chanserv_unignore,		{"UNIGNORE", NULL, NULL, NULL, NULL}, "<nick>"},
    {INFO_COMMAND,   -1, 1, chanserv_unixtime,		{"UNIXTIME", NULL, NULL, NULL, NULL}, "<time>"},
#if DO_CHANBOT_CRAP == 1
    {DANGER_COMMAND,  2, 0, chanserv_up,		{"UP", NULL, NULL, NULL, NULL}, NULL},
#endif
#ifndef	WIN32
    {INFO_COMMAND,   -1, 0, chanserv_uptime,		{"UPTIME", NULL, NULL, NULL, NULL}, NULL},
#endif
    {INFO_COMMAND,    1, 0, chanserv_user_list,		{"USERLIST", "HLIST", "ACCESS", NULL, NULL}, NULL},
#if STATUS == 1
    {INFO_COMMAND,    1, 0, chanserv_users_list,	{"LUSERS", NULL, NULL, NULL, NULL}, NULL},
#endif
    {INFO_COMMAND,   -1, 0, chanserv_variables,		{"VARIABLES", NULL, NULL, NULL, NULL}, NULL},
#if CTCP == 1
    {INFO_COMMAND,   -1, 0, chanserv_version,		{"\1VERSION\1", NULL, NULL, NULL, NULL}, NULL},
#endif
    {DANGER_COMMAND,  3, 1, chanserv_vhost,		{"VHOST", "SETHOST", NULL, NULL, NULL}, "<new virtual host> (requires a restart)"},
#if DO_CHANBOT_CRAP == 1
    {DANGER_COMMAND,  3, 1, chanserv_voice,		{"VOICE", "V", NULL, NULL, NULL}, "<nick>"},
#endif
    {SAFE_COMMAND, SLEEP_LEVEL, 0, chanserv_wakeup,	{"WAKEUP", NULL, NULL, NULL, NULL}, NULL},
    {NORMAL_COMMAND, -1, 1, chanserv_weather,		{"WEATHER", NULL, NULL, NULL, NULL}, "<city or code>"},
    {INFO_COMMAND,   -1, 2, chanserv_where,		{"WHERE", "WHO", "WHAT", NULL, NULL}, "<IS> [A|AN] <topic>"},
    {SAFE_COMMAND,   -1, 2, chanserv_whisper,		{"WHISPER", NULL, NULL, NULL, NULL}, "<nick> [ABOUT] <topic>"},
    {NORMAL_COMMAND, -1, 0, NULL, {NULL, NULL, NULL, NULL, NULL}, NULL}
};

void chanserv(char *source, char *target, char *buf)
{
	struct chanserv_output *result = NULL;
	char *cmd = NULL, *userhost = NULL;
	int i, j, found = -1, command = 0, wakeup = 0;
	enum chanserv_invoke_type input_type = DIRECT_INVOKE;
	enum chanserv_command_type command_type = NORMAL_COMMAND;

#ifdef	RANDOM_STUFF
	if (stricmp (target, CHAN) == 0)
		Rand_Idle = 0;
#endif
	stripline (buf);
	stripline (source);
	if (buf == NULL || target == NULL || source == NULL)
	    return;
	cmd = strtok (buf, " ");
	if (cmd == NULL)
	    return;
	if (*cmd == ':')
	    cmd++;
	if ((userhost = strchr (source, '!')) != NULL)
	    *userhost++ = '\0';

	if (check_ignore_user_ram(source) > 0)
    	    return;

	if (*target != '#' && *target != '&' && *target != '+')
		input_type = MSG_INVOKE;
	else if (stricmp (cmd, NICK_COMMA) == 0 || stricmp (cmd, COLON_NICK) == 0 || stricmp (cmd, BCOLON_NICK) == 0 || stricmp (cmd, Mynick) == 0)
	{
		input_type = ADDRESS_INVOKE;
		cmd = strtok(NULL, " ");
	}
	else if (*cmd == *CMDCHAR)
		input_type = CHAR_INVOKE;

	if (cmd != NULL)
	{
	    if (*cmd == *CMDCHAR)
	    {
		cmd++;
		command = 1;
	    }

	    strupr(cmd);
    	    for (i = 0; chanserv_commands[i].func != NULL; i++)
	    {
		for (j = 0; chanserv_commands[i].command[j] != NULL; j++)
		{
		    if (strcmp(cmd, chanserv_commands[i].command[j]) == 0)
		    {
			found = i;
			command_type = chanserv_commands[found].type;
			if (strcmp(cmd, "WAKEUP") == 0)
			    wakeup = 1;
	    		break;
		    }
		}
		if (found != -1)
		    break;
	    }
	}

	if (input_type != MSG_INVOKE)
	{
	    if (((Sleep_Toggle == 1) && (wakeup != 1)) || (cf(userhost, source, target)))
		return;
    	    add_user(target, source, userhost, 0);	/* Unidle */
	}

	if (found != -1)
	{
	    /*
	     * Every             command can be done by             !command.
	     * Every             command can be done by             'command.
	     * Every normal      command can be done by /query bot   command.
	     * Every normal      command can be done by /msg   bot   command.
	     * Every safe        command can be done by        bot:  command.
	     * Every information command can be done by              command.
	     */

	    switch (command_type)
	    {
		case INFO_COMMAND :
			break;

		case SAFE_COMMAND :
		    {
			if (input_type == DIRECT_INVOKE)
			    return;
			break;
		    }

		case NORMAL_COMMAND :
		    {
			if (input_type == DIRECT_INVOKE)
			    return;
			else if (input_type == ADDRESS_INVOKE)
			    return;
			break;
		    }

		case DANGER_COMMAND :
		    {
			if (command != 1)
			    return;
			break;
		    }
	    }

	    if (check_access(userhost, (input_type == MSG_INVOKE) ? "#*" : target, 0, source) >= chanserv_commands[found].access)
	    {
		char **args = NULL;
		int more_needed = 0;

		j = chanserv_commands[found].arg_count;
		if (j > 0)
		{
		    args = calloc(j, sizeof(char *));
		    if (args)
		    {
			for (i = 0; i < j; i++)
			{
			    args[i] = strtok(NULL, " ");
			    if (args[i] == NULL)
			    {
				more_needed = 1;
				break;
			    }
			}
		    }
		    else  // FIXME: Should bitch about lack of ram.
			return;
		}
		/* We call this first to give the command a chance to supply a custom error msg if there are not enough arguments. */
    		result = chanserv_commands[found].func(source, target, cmd, args, input_type, userhost);

		if (result)
		{
		    struct chanserv_output *output = result;

		    while (output)
		    {
			if (input_type == MSG_INVOKE)
			    S("NOTICE %s :%s\n", source, output->output);
			else
			    S("PRIVMSG %s :%s: %s\n", target, source, output->output);
			output = output->next;
		    }
		    chanserv_output_free(result);
		}
		else if (more_needed && (chanserv_commands[found].syntax))
		{
			if (input_type == MSG_INVOKE)
			    S("NOTICE %s :SYNTAX - %s%s %s\n", source, command ? "!" : "", cmd, chanserv_commands[found].syntax);
			else
			    S("PRIVMSG %s :%s: SYNTAX - %s%s %s\n", target, source, command ? "!" : "", cmd, chanserv_commands[found].syntax);
		}

		free(args);
	    }
	}
	else if (input_type != CHAR_INVOKE)
	{
	    if ((input_type == DIRECT_INVOKE) && (cmd == NULL))
	    {
#if RANDOM_WHUT == 1
		do_randomtopic(WHUTR, target, WHUT_FILE, source, cmd);
#else
		if (input_type == MSG_INVOKE)
		    S("NOTICE %s :%s\n", source, WHUT);
		else
		    S("PRIVMSG %s :%s: %s\n", target, source, WHUT);
#endif
	    }
#if GENERAL_QUESTIONS == 1
	    else if (cmd != NULL)
	    {
		show_url(source, get_multiword_topic(cmd), 
		    (input_type == MSG_INVOKE) ? source : target, 
		    (! (input_type == DIRECT_INVOKE)), 
		    (input_type == DIRECT_INVOKE), 
		    userhost, 0);
	    }
#endif
	}
}

struct chanserv_output *chanserv_show_help(char *cmd)
{
	// FIXME: Add a short summary of what the command does.
	// FIXME: Add a "list commands" help option.
	struct chanserv_output *result = NULL;
	int i, j, found = -1;

	strupr(cmd);
    	for (i = 0; chanserv_commands[i].func != NULL; i++)
	{
	    for (j = 0; chanserv_commands[i].command[j] != NULL; j++)
	    {
		if (strcmp(cmd, chanserv_commands[i].command[j]) == 0)
		{
		    found = i;
	    	    break;
		}
	    }
	    if (found != -1)
		break;
	}

	if (found != -1)
	{
	    if (chanserv_commands[found].syntax)
		result = chanserv_asprintf(result, "SYNTAX - %s %s", cmd, chanserv_commands[found].syntax);
	    else
		result = chanserv_asprintf(result, "SYNTAX - %s", cmd);
	}
	return result;
}
