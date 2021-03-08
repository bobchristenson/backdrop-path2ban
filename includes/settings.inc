<?php

/**
 * @file
 * path2ban settings file.
 */

/**
 * Display path2ban settings form.
 */
function path2ban_settings() {
  $form = array();
  $config = config('path2ban.settings');
  $banned_count = (int) $config->get('path2ban_banned_count') ?: 0;
  $banned_since = format_date($config->get('path2ban_banned_since') ?: '[run the database update]', 'medium');
  $form['#config'] = 'path2ban.settings';
  $form['path2ban_ban_count'] = array(
    '#markup' => t(
      'Path2ban has banned @banned_count visitors since @banned_since.',
      array('@banned_count' => $banned_count, '@banned_since' => $banned_since)
    ),
  );

  $form['path2ban_options'] = array(
    '#type' => 'fieldset',
    '#title' => t('path2ban options'),
  );
  $description = t(
    "Specify pages by using their paths. Enter one path per line. The '*' character is a wildcard. Example paths are %blog for the blog page and %blog-wildcard for every personal blog. %front is the front page.",
    array('%blog' => 'blog', '%blog-wildcard' => 'blog/*', '%front' => '<front>')
  );
  $form['path2ban_options']['path2ban_list'] = array(
    '#type' => 'textarea',
    '#title' => t('List of restricted paths'),
    '#description' => $description,
    '#default_value' => $config->get('path2ban_list') ?: '',
    '#rows' => 25,
  );
  $form['path2ban_options']['path2ban_threshold_limit'] = array(
    '#type' => 'textfield',
    '#title' => t('Threshold limit'),
    '#description' => t('The number of times an IP address can access restricted URLs during the threshold window before they are banned.'),
    '#default_value' => $config->get('path2ban_threshold_limit') ?: 5,
  );
  $form['path2ban_options']['path2ban_threshold_window'] = array(
    '#type' => 'textfield',
    '#title' => t('Threshold window'),
    '#description' => t('The time period in which to consider attacks. 3600 seconds is one hour.'),
    '#default_value' => $config->get('path2ban_threshold_window') ?: 3600,
  );
  $form['path2ban_options']['path2ban_notify'] = array(
    '#type' => 'checkbox',
    '#title' => t('Notify user one?'),
    '#default_value' => $config->get('path2ban_notify') ?: 0,
    '#description' => t('Notify user one by email about blocked IP addresses.'),
  );
  $form['path2ban_options']['path2ban_warn_user'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show warning message?'),
    '#default_value' => $config->get('path2ban_warn_user') ?: 0,
    '#description' => t('Warns the user that they are attempting to access restricted paths, and may be IP banned.'),
  );
  $form['path2ban_options']['path2ban_warn_user_message'] = array(
    '#type' => 'textfield',
    '#title' => t('Warning message'),
    '#default_value' => $config->get('path2ban_warn_user_message') ?: 'Access to this page is restricted.<br/>If you continue to attempt to access it you will be blocked from this website.',
    '#description' => t('The message to show the user if the "Warning message" box is checked.'),
  );

  $form['path2ban_options']['path2ban_mode'] = array(
    '#type' => 'value',
    '#value' => Path2ban_SettingsManager::MODE_USE_MENU_CALLBACK,
    /*'#title' => t('Mode'),
    '#default_value' => $config->get('path2ban_mode') ?: Path2ban_SettingsManager::MODE_USE_MENU_CALLBACK,
    '#options' => array(
      Path2ban_SettingsManager::MODE_USE_MENU_CALLBACK => t('Use the 403
        and 404 error pages (old). Choosing this will overwrite these settings.
        You can check what they are
        <a href="/admin/config/system/site-information">here</a>.'),
      Path2ban_SettingsManager::MODE_USE_HOOK => t('Use hooks (new). Selecting
        this option means that path2ban will still work even if other modules,
        or your own configuration, use Drupal\'s 403 and 404 routes.'),
    ),*/
  );

  $form['#submit'][] = 'path2ban_settings_form_submit';

  return system_settings_form($form);
}

/**
 * Implements hook_page_alter().
 *
 * If watchdog is available, includes the lists of recent 403 and 404 errors.
 */
function path2ban_page_alter(&$page) {
  if ('admin/config/people/path2ban' !== $_GET['q']) {
    return;
  }

  if (!user_access('administer path2ban')) {
    return;
  }

  // Some sites disable watchdog.
  if (!module_exists('dblog')) {
    $page['content']['no_blog_warning'] = array(
      '#markup' => '<div>On sites with Drupal watchdog enabled, we include a list
        of recent 403 and 404 errors here. We recommend you review your operating
        system\'s logs to discover new paths to restrict.</div><br/>',
    );
    return;
  }

  // At this point we know that watchdog is enabled.
  module_load_include('inc', 'dblog', 'dblog.admin');

  $page['content']['path2ban_error_list'] = array(
    '#type' => 'fieldset',
    '#title' => t('Recent 404 errors'),
  );

  $page['content']['path2ban_error_list']['intro'] = array(
    '#markup' => '<div>This is a list of recent 404 errors from watchdog.
      You can use these to decide which paths to restrict using the field above.
      <br/><br/>
      Occasionally legitimate visitors may get links wrong, or may
      even attempt to find out what CMS you\'re using. Therefore it\'s
      usually best not to block every item that appears in this list.<br/><br/>
      In most cases it\'s probably best not to restrict visitors based on access
      denied errors, as those can occur when users are automatically logged out,
      but you can find a list of those
      <a href="/admin/reports/access-denied">here</a>.</div><br/>',
  );

  $page['content']['path2ban_error_list']['404_header'] = array(
    '#markup' => '<strong>404 errors</strong><br/>',
  );

  $page['content']['path2ban_error_list']['404_errors'] = dblog_top('page not found');
}

/**
 * Implements settings form submit callback.
 */
function path2ban_settings_form_submit($form, &$form_state) {
  // Only switch modes if there is a difference.
  if ($form_state['values']['path2ban_mode'] != config_get('path2ban.settings', 'path2ban_mode')) {
    switch ($form_state['values']['path2ban_mode']) {
      case Path2ban_SettingsManager::MODE_USE_MENU_CALLBACK:
        Path2ban_SettingsManager::switchToMenuCallbackMode();
        break;
      case Path2ban_SettingsManager::MODE_USE_HOOK:
        Path2ban_SettingsManager::switchToHookMode();
        break;
    }
  }
}
