<?php

/**
 * @file
 * Drush integration for the CultuurNet API shadow nodes module
 */

/**
 * Implements hook_drush_help().
 */
function cnapi_ui_drush_help($section) {
  switch ($section) {
    case 'cnapi:cnapi-ui-register-deletions':
      return dt("Store deleted cdbis in a table, so we can trigger 410's.");
  }
}


/**
 * Implements hook_drush_command().
 */
function cnapi_ui_drush_command() {
  $items['cnapi-ui-register-deletions'] = array(
    'description' => 'Store deleted cdbids.',
  );

  return $items;
}

/**
 * Implements the 'cnapi-snode-cleanup' drush command.
 */
function drush_cnapi_ui_register_deletions() {

  $last_registration = variable_get('cnapi_ui_last_registration', 0);
  //$last_registration = 0;
  $query = array('deletedsince' => date('Y-m-d', $last_registration));

  $types = array('actor', 'event', 'production');

  foreach ($types as $type) {
    watchdog('cnapi_ui', 'Fetching deleted objects of type !type from API for cleanup.', array('!type' => $type), WATCHDOG_INFO);

    $request = array('action' => 'list_detail', 'type' => $type, 'query' => $query);

    $result = cnapi_get($request);
    if (!$result) {
       watchdog('cnapi_ui', 'Failed registration of deleted objects for type !type. Stopped trying for all types and will retry again on the next run.', array('!type' => $type), WATCHDOG_ERROR);
      return;
    }

    if (isset($result['deletes'])) {
      watchdog('cnapi_ui', 'Registering deletions from !type from API.', array('!type' => $type), WATCHDOG_INFO);

      $deletions = array_unique($result['deletes']);
      foreach ($deletions as $cdbid) {

        $existing = db_query('SELECT cdbid FROM {cnapi_deleted_objects} WHERE cdbid = :cdbid', array(':cdbid' => $cdbid))->fetchField();
        $update = ($existing) ? 'cdbid' : array();

        $record = array();
        $record['cdbid'] = $cdbid;
        $record['deleted'] = $_SERVER['REQUEST_TIME'];
        $record['type'] = $type;

        drupal_write_record('cnapi_deleted_objects', $record, $update);

      }
    }
  }

  watchdog('cnapi_ui', 'Finished registration of deleted objects.', array('!type' => $type), WATCHDOG_INFO);

  variable_set('cnapi_ui_last_registration', REQUEST_TIME);

}
