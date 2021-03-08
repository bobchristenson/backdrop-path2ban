<?php

/**
 * @file
 * To contain the path2ban settings manager class
 */

/**
 * @class
 * Path2ban_SettingsManager is a container class to store the functionality that
 * manages Path2ban's settings.
 */
abstract class Path2ban_SettingsManager {

  const MODE_USE_MENU_CALLBACK = 1;
  const MODE_USE_HOOK = 2;

  /**
   * A utility function to add new entries to the restricted paths list.
   *
   * @param array $new_entries
   */
  public static function addNewEntries($new_entries) {
    $config = config('path2ban.settings');
    $list = $config->get('path2ban_list') ?: path2ban_get_default_paths_to_ban();
    $list = $list . "\n";

    // Check that the user hasn't already added them before adding.
    foreach ($new_entries as $each_new_entry) {
      if (FALSE === strpos($list, $each_new_entry)) {
        $list .= $each_new_entry . "\n";
      }
    }

    $config->set('path2ban_list', $list);
    $config->save();
  }

  /**
   * Switch to hook mode, clearing the site_403 and site_404 variables as we go.
   */
  public static function switchToHookMode() {
    config_set('path2ban.settings', 'path2ban_mode', self::MODE_USE_HOOK);

    // Wipe old path variables.
    if ('path2ban/403' == config_get('system.core','site_403')) {
      config_set('system.core','site_403', '');
    }
    if ('path2ban/404' == config_get('system.core','site_404')) {
      config_set('system.core','site_404', '');
    }
  }

  /**
   * Switch to the old mode, which needs the site_403 and site_404 variables to
   * be set.
   */
  public static function switchToMenuCallbackMode() {
    config_set('path2ban.settings', 'path2ban_mode', self::MODE_USE_MENU_CALLBACK);

    // Log what the old settings were.
    $old_site_403 =  config_get('system.core','site_403');
    $old_site_404 =  config_get('system.core','site_404');

    // Update the settings to the new values.
    config_set('system.core','site_403', 'path2ban/403');
    config_set('system.core','site_404', 'path2ban/404');

    // Show the user messages and log the old entries if needed.
    $variables_changed = FALSE;

    if (!in_array($old_site_403, array('', 'path2ban/403'))) {
      $variables_changed = TRUE;
    }
    if (!in_array($old_site_404, array('', 'path2ban/404'))) {
      $variables_changed = TRUE;
    }

    if (TRUE == $variables_changed) {
      if (module_exists('dblog')) {
        watchdog(
          'path2ban',
          'Path2ban updated your site_403 and site_404 settings. site_403 was
            \'%old_site_403\' and site_404 was \'%old_site_404\'.',
          array(
            '%old_site_403' => $old_site_403,
            '%old_site_404' => $old_site_404
          ),
          WATCHDOG_WARNING
        );
        backdrop_set_message("Path2ban has overwritten your site 403 and 404
          paths.\n The old entries can be found in your watchdog log.");
      }
      else {
        backdrop_set_message('Your site 403 and 404 paths were overridden.');
      }
    }

  }

  /**
   * Check to ensure that there's always a mode selected, and if that mode is
   * the menu callback that it has the correct callback paths.
   */
  public static function checkRuntimeRequirements() {

    $mode =  config_get('path2ban.settings', 'path2ban_mode');

    $return_values = array('path2ban_mode_check' => array('title' => 'Path2ban mode'));

    if (self::MODE_USE_HOOK == $mode) {
      $return_values['path2ban_mode_check']['value'] = t('Hook mode selected');
      $return_values['path2ban_mode_check']['severity'] = REQUIREMENT_OK;
    }
    elseif (self::MODE_USE_MENU_CALLBACK == $mode) {
      $return_values['path2ban_mode_check']['value'] = t('Menu callback mode selected');
      $return_values['path2ban_mode_check']['severity'] = REQUIREMENT_OK;

      // Check the site_403 and site_404 variables.
      $site_current_403 = config_get('system.core','site_403');
      $site_current_404 = config_get('system.core','site_404');

      $value = t('site_403: :site_current_403 site_404: :site_current_404',
        array(
          ':site_current_403' => $site_current_403,
          ':site_current_404' => $site_current_404,
        )
      );

      $return_values['path2ban_menu_callback_check'] = array(
        'title' => t('Path2ban menu callback settings'),
        'value' => $value,
      );

      if ($site_current_403 == 'path2ban/403' && $site_current_404 == 'path2ban/404') {
        $return_values['path2ban_menu_callback_check']['severity'] = REQUIREMENT_OK;
      }
      else {
        $return_values['path2ban_menu_callback_check']['description'] = t('For Path2ban to work in menu callback mode, the site_403 and site_404 settings <a href="/admin/config/system/site-information">here</a> must be set to \'path2ban/403\' and \'path2ban/404\'.<br/>Alternatively switch Path2ban to \'use hook\' on the Path2ban <a href="/admin/config/people/path2ban">config page</a>.');
        $return_values['path2ban_menu_callback_check']['severity'] = REQUIREMENT_ERROR;
      }
    }
    else {
      $return_values['path2ban_mode_check']['value'] = t('Invalid mode setting');
      $return_values['path2ban_mode_check']['description'] = t('Path2ban doesn\'t currently have a valid mode selected. To resolve this, select a mode on the Path2ban <a href="/admin/config/people/path2ban">config page</a>.');
      $return_values['path2ban_mode_check']['severity'] = REQUIREMENT_ERROR;
    }

    return $return_values;
  }

}
