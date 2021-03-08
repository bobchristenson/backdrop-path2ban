<?php

/**
 * @file
 * path2ban menu file.
 */

/**
 * Implements hook_menu().
 */
function path2ban_menu() {
  $menu = array();
  $menu['path2ban/%'] = array(
    'page callback' => 'path2ban_menu_callback',
    'page arguments' => array(1),
    'access callback' => TRUE,
    'type' => MENU_CALLBACK,
  );

  $menu['admin/config/people/path2ban'] = array(
    'title' => 'path2ban',
    'description' => 'Ban IP of visitors, who try to scan restricted paths.',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('path2ban_settings'),
    'access arguments' => array('administer path2ban'),
    'type' => MENU_NORMAL_ITEM,
  );
  return $menu;
}

/**
 * This menu_callback arrives from either the 403 or 404 responses, so it doesn't
 * have to check if the path is valid.
 */
function path2ban_menu_callback($state = 404) {
  Path2ban::destinationCheck();
  backdrop_page_is_cacheable(FALSE);
  return ($state == 403) ? system_403() : system_404();
}
