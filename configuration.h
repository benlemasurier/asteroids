#ifndef asteroids_configuration_H
#define asteroids_configuration_H

#define CONFIG_FILE "asteroids.ini"

const char *get_config_value(const char *key);
bool set_config_value(const char *key, const char *value);

#endif /* asteroids_configuration_H */
