#ifndef __HOMER_H__
#define __HOMER_H__
/**
 *          |    _________
 *          |   /  mmmmm, \
 *          |   \doughnuts/   _
 *          |        O      _//\-\
 *          |         O    /      \
 *         _|_         o  /       |
 *        /   \         (.(.) /|\/
 *       |  0  |         (___    ,)
 *        \___/          /   \   \
 *            _          \o  /   |
 *          _( \_         _| _____\
 *         (___  \_______/\_/______\
 *         (___         /    /    \|
 *         (___________/     |____||
 *                    /      |    ||
 *                   /_______|    |_\
 *                   \      _|    | /
 *                    |    (_     \/
 *                    | \__  | | | |
 *                    |    \ |_|_|_|
 *                    |     |     |
 *                    |     |     |
 *                    |     |     |
 *                    |_____|_____|
 *                    |_____|_____|
 *                   /     /      |
 */

#include "stepper.h"
#include "sys_config.h"
#include "utils.h"

#include <stdbool.h>

void Homer_Init(Stepper_t* mx, Stepper_t* my);

void Homer_Update(void);

void Homer_HomingStart(void);

bool Homer_IsDone(void);

#endif /* __HOMER_H__ */