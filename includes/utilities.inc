<?php

/**
 * @file
 * Set of utility functions that are widely used throughout the module code.
 */

/**
 * This will copy the FRX and SQL files from the forena_reports/ folder into the
 * forena configured folder locations so they are able to be used after installing.
 *
 * This will also be called from the admin interface so that the reports would be
 * merged at that point in time ONLY if they haven't been deployed yet.
 *
 * @return BOOLEAN
 *   TRUE when the deployment succeeds to move the files or if it was already
 * deployed.
 */
function islandora_object_validation_check_and_deploy_forena_reports() {
  if (!module_exists('forena')) {
    $link_att = array('attributes'=>array(
      'title' => 'link opens in separate tab',
      'class' => array('link_open_new_tab_tiny'),
      'target' => '_blank'));
    drupal_set_message(t('The Forena reporting module is not installed.  ') .
        l('Forena', 'https://git.drupal.org/project/forena.git', $link_att) . ' | ' .
        l('ulsdevteam "Forena"', 'https://github.com/ulsdevteam/forena.git', $link_att), 'error'
    );
    return FALSE;
  }

  // Get the forena report FRX and SQL file deployment locations.
  module_load_include('module', 'forena', 'forena');

  $forena_frx_path = forena_report_path();
  $forena_data_source_idw = Frx::RepoMan()->repository('islandora_object_validation_sql');
  if (is_object($forena_data_source_idw) && $forena_frx_path) {
    $forena_sql_path = $forena_data_source_idw->block_path;
    $frx_files = islandora_object_validation_scan_files($forena_frx_path);
    $sql_files = islandora_object_validation_scan_files($forena_sql_path);

    // Finally set up the file paths needed if deploying.
    $this_folder = dirname(__FILE__);
    @list($sites_location, $junk) = explode("sites/", $this_folder);
    // Since $this_folder is going to end with /includes, strip that off
    $this_folder = str_replace("/includes", "", $this_folder);

    // Perform the file deployment from the islandora_object_validation/forena_reports
    // folder locations into the FRX and SQL folders.
    // Only deploy the FRX files if they do not exist
    if ((array_key_exists('validation_queue.frx', $frx_files) === FALSE) ||
        (array_key_exists('validation_report.frx', $frx_files) === FALSE)) {
      $from =  $this_folder . '/forena_reports/frx';
      $to = $sites_location . $forena_frx_path;
      islandora_object_validation_deploy_files_to_forena_install($from, $to);
      drupal_set_message(t('Deployed Forena FRX files to: "@to"<br>' .
          'from: "@from".', array(
              '@to' => $to,
              '@from' => $from,
          )), 'message_info');
    }
    // Only deploy the SQL files if they do not exist
    if ((array_key_exists('validation_queue.sql', $sql_files) === FALSE) ||
        (array_key_exists('validation_report.sql', $sql_files) === FALSE)) {
      $from = $this_folder . '/forena_reports/sql';
      $to = $sites_location . $forena_sql_path;
      islandora_object_validation_deploy_files_to_forena_install($from, $to);
      drupal_set_message(t('Deployed Forena SQL files to: "@to"<br>' .
          'from: "@from".', array(
              '@to' => $to,
              '@from' => $from,
          )), 'message_info');
    }
  }
  else {
    // the 'islandora_object_validation_sql' forena data source did not exist --
    // create it here and display the message that the page needs to reload.
    drupal_set_message(t('The "islandora_object_validation_sql" Forena data ' .
        'source is not configured and will be created.  You will need to ' .
        'reload this page in order to complete the deployment of the Islandora ' .
        'Object Validation reports and SQL files.'), 'error');
    islandora_object_validation_create_islandora_object_validation_sql_configuration();
    return FALSE;
  }
  return TRUE;
}

/**
 * Inserts the record into the forena_repositories table.
 */
function islandora_object_validation_create_islandora_object_validation_sql_configuration() {
  $private_path = rtrim(variable_get('file_private_path', ''), '/') . '/';
  if (!$private_path) {
    drupal_set_message(t('The Drupal | Media | File System "Private file system ' .
        'path" is not configured.  This must be configured and the folder must' .
        'be writable by Drupal and not accessible over the web.'), 'error');
    return FALSE;
  }
  $config_array = array(
    'source' => 'user',
    'data provider' => 'FrxDrupal',
    'database' => 'default',
    'access callback' => 'user_access',
    'user callback' => 'forena_current_user_id',
    'path' => $private_path . 'islandora_object_validation_sql',
    'debug' => 0
  );
  // Make the value serialized like:  //  a:7:{s:6:"source";s:4:"user";s:13:"...
  $config_str = serialize($config_array);
  db_insert('forena_repositories')
    ->fields(array(
        'repository' => 'islandora_object_validation_sql',
        'title' => 'Islandora Object Validation Reports',
        'enabled' => 1,
        'config' => $config_str,
    ))
    ->execute();
}

/**
 * This will create the destination folder if needed (if possible), and copy
 * all files from the "$from" to the "$to"
 *
 * @param string $from
 *   The folder from which the files are deployed.
 * @param string $to
 *   The folder to where the folders are deployed.
 */
function islandora_object_validation_deploy_files_to_forena_install($from, $to) {
  // make sure that the destination and source folders do not end with "/" just in case.
  $to = rtrim($to, '/');

  if (!file_exists($to)) {
    $success = mkdir($to);
    if (!$success) {
      drupal_set_message(t('The destination folder did not exist and could not ' .
          'be created.  Please check the file permissions of the parent folder for "@to".', array(
              '@to' => $to
          )), 'error');
    }
    return FALSE;
  }
  @list($junk, $keep_from) = explode("sites/", $from);
  $files = islandora_object_validation_file_scandir('sites/' . $keep_from, '', TRUE);
  $from = rtrim($from, '/');
  $success = FALSE;
  foreach ($files as $file) {
    $success &= copy($from . '/' . $file, $to . '/' . $file);
  }
  if (!$success) {
      drupal_set_message(t('The forena files could not be deployed due to permissions ' .
          'on the folder.  Please check the file permissions of the parent folder for "@to".', array(
              '@to' => $to
          )), 'error');
  }
  return $success;
}

/**
 * Helper function to return a set of filenames from a given path -- with the option to filter ($mask)
 * and an option to recursively ($recurse) search for files under the initial directory path ($dir).
 */
function islandora_object_validation_file_scandir($dir, $mask = '', $recurse = FALSE, $returnOnlyFolders = FALSE) {
  $result = array();
  if (!is_dir($dir)) {
    return $result;
  }
  $cdir = scandir($dir);
  foreach ($cdir as $key => $value) {
    if (!in_array($value,array(".",".."))) {
      if ($recurse && (is_dir($dir . DIRECTORY_SEPARATOR . $value))) {
        $result[$value] = islandora_object_validation_file_scandir($dir . DIRECTORY_SEPARATOR . $value, $mask, $recurse, $returnOnlyFolders);
      }
      elseif (!$recurse || !$returnOnlyFolders) {
        if (!$mask || (strstr($value, $mask))) {
          $result[] = $value;
        }
      }
    }
  }
  return $result;
}

/**
 * This will return the set of files that are found in the $scan_path.
 *
 * @param string $scan_path
 *   Folder to scan.
 * @return array
 *   An array of the files that reside in the $scan_path that relate to the
 * object underlying that folder.
 */
function islandora_object_validation_scan_files($scan_path) {
  $files = islandora_object_validation_file_scandir($scan_path, array(), TRUE);
  $ret_arr = array();
  $sequence = 1;
  // Join together the $filters and the $file_masks['discard_filemasks']
  foreach ($files as $folder => $folder_files) {
    if (is_array($folder_files)) {
      foreach ($folder_files as $subfolder => $file) {
        $subfolder = (is_array($file)) ? $subfolder . "/" : '';
        $file = (is_array($file)) ? array_pop($file) : $file;
        $use_folder = ($subfolder) ? $folder . '/' . $subfolder : $folder;
        $ret_arr[$folder . '/' . $subfolder . $file] = $folder . '/' . $subfolder . $file; //islandora_object_validation_get_filesize_class($scan_path, $use_folder, $file, FALSE, $sequence);
      }
    }
    else {
      $ret_arr[$folder_files] = $folder_files; // islandora_object_validation_get_filesize_class($scan_path, '', $folder_files, FALSE, $sequence);
    }
  }
  ksort($ret_arr);

  return $ret_arr;
}
