#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <allegro5/allegro.h>

#include "configuration.h"

ALLEGRO_CONFIG *config = NULL;

static void
load_config(void)
{
  if((config = al_load_config_file(CONFIG_FILE)) == NULL) {
    config = al_create_config();
    al_save_config_file(CONFIG_FILE, config);
  }
}

const char *
get_config_value(const char *key)
{
  if(!config)
    load_config();

  return al_get_config_value(config, NULL, key);
}

bool
set_config_value(const char *key, const char *value)
{
  if(!config)
    load_config();

  al_set_config_value(config, NULL, key, value);

  return al_save_config_file(CONFIG_FILE, config);
}
