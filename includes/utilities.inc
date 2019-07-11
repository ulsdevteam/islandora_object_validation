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

function islandora_object_validation_get_pids_solr_query($solr_query) {
  return implode('
', islandora_object_validation_get_pids($solr_query));
}

function islandora_object_validation_get_pids_collection($collection) {
  $query = format_string('!field:!value', array(
    '!field' => 'RELS_EXT_isMemberOfCollection_uri_ms',
    '!value' => str_replace(array("/", ":", "-", "+"), array("\/", "\:", "\-", "\+"), $collection),
   ));
  return implode('
', islandora_object_validation_get_pids($query));
}

function islandora_object_validation_get_pids_model($model) {
  $query = format_string('!field:!value', array(
    '!field' => 'RELS_EXT_hasModel_uri_ms',
    '!value' => str_replace(array("/", ":", "-", "+"), array("\/", "\:", "\-", "\+"), $model),
   ));
  return implode('
', islandora_object_validation_get_pids($query));
}

function islandora_object_validation_get_pids($query) {
  module_load_include('inc', 'islandora_solr', 'includes/utilities');
  $query_processor = new IslandoraSolrQueryProcessor();
  $ret_arr = array();
  // run two queries --
  // 1) to get the datastreams available on this object
  $query_processor->solrQuery = $query;
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = variable_get('islandora_object_validation_solr_limit', 10000);
  $query_processor->solrParams = array('fl' => 'PID');

  $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
  $solr->setCreateDocuments(FALSE);
  try {
    $search_results = $solr->search($query_processor->solrQuery, $query_processor->solrStart, $query_processor->solrLimit, $query_processor->solrParams, 'GET');
    $tmp = json_decode($search_results->getRawResponse(), TRUE);

    $results = array();
    $numFound = $tmp['response']['numFound'];
    if ($numFound == variable_get('islandora_object_validation_solr_limit', 10000)) {
      drupal_set_message(t('The number of objects returned is the same as the maximum limit.  It is likely that there are more objects than ' . variable_get('islandora_object_validation_solr_limit', 10000)));
    }

    if ($tmp['response']['numFound'] > 0) {
      foreach ($tmp['response']['docs'] as $k=>$rec) {
        $ret_arr[] = $rec['PID'];
      }
    }
  }
  catch (Exception $e) {
    error_log('EXCEPTION in islandora_object_validation_get_pids : called from ' . $_SERVER['SERVER_NAME'] .
' - ' . $_SERVER['REQUEST_URI'] . '
' . print_r($e, true));
  }
  return $ret_arr;
}

function islandora_object_validation_get_models() {
  form_load_include($form_state, 'inc', 'islandora', 'includes/content_model.autocomplete');

  // Retrieve all content models and unset those currently in use in this
  // configuration and any others from other configurations.
  $add_options = islandora_get_content_model_names();
  return $add_options;
}

function islandora_object_validation_get_fetch_methods($pid_values) {
  $fetch_methods = array(
      ISLANDORA_OBJECT_VALIDATION_FETCH_LISTPIDS => 'List of PID values' .
        ((count($pid_values) > 0) ? ' (' . count($pid_values) . ' PID value' .
        ((count($pid_values) == 1) ? '' : 's') . ' supplied)' : ''));
  if (module_exists('islandora_solr')) {
    $fetch_methods[ISLANDORA_OBJECT_VALIDATION_FETCH_SOLR] = 'Solr query';
    $fetch_methods[ISLANDORA_OBJECT_VALIDATION_FETCH_COLLECTION] = 'Collection';
    $fetch_methods[ISLANDORA_OBJECT_VALIDATION_FETCH_MODEL] = 'Model';
  }
  return $fetch_methods;
}

function islandora_object_validation_get_solr_options($solr_query, $key_field, $value_field, $sort_field = '') {
  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->solrQuery = $solr_query;
  if ($sort_field) {
    $query_processor->sort = $sort_field;
  }
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = 40000;
  $query_processor->solrParams = array('fl' => $key_field . ',' . $value_field);

  $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
  $solr->setCreateDocuments(FALSE);
  $results = array();
  try {
    $search_results = $solr->search($query_processor->solrQuery, $query_processor->solrStart, $query_processor->solrLimit, $query_processor->solrParams, 'GET');
    $tmp = json_decode($search_results->getRawResponse(), TRUE);
    if ($tmp['response']['numFound'] > 0) {
      foreach ($tmp['response']['docs'] as $rec) {
        $results[$rec[$key_field]] = $rec[$value_field];
      }
    }
  }
  catch (Exception $e) {
  }
  return $results;
}

function islandora_object_validation_validate($object) {
  // Perform the validation check on this object only if this is configured to happen.
  if (variable_get('islandora_object_validation_validate_on_ingest', 0) && is_object($object)) {
    module_load_include('inc', 'islandora_object_validation', 'includes/db');
    $object_models = $object->models;
    $check_dsids = array();
    $validation_datastreams = variable_get('islandora_object_validation_selected_cmodels', array());
    foreach ($object_models as $object_model) {
      $lc_name = str_replace("islandora:", "", strtolower($object_model));
      if (strstr($object_model, 'fedora-system:') == '' && $lc_name) {
        $lc_name = str_replace("islandora:", "", strtolower($object_model));
        foreach ($validation_datastreams as $key => $validation_datastream) {
          if (strstr($key, $lc_name) <> '') {
            $dsid = str_replace($lc_name . '_', "", $key);
            $check_dsids[$dsid] = $dsid;
          }
        }
      }
    }
    $good_dsids = $successes = $failures = array();
    // Need to check these datastreams for validity.
    foreach ($object as $dsid => $fedoraDatastream) {
      if (!(array_search($dsid, $check_dsids) === FALSE)) {
        $model_has_datastream = FALSE;
        try {
          $model_has_datastream = $object[$dsid]; // ->content;
          $good_dsids[$dsid] = $dsid;
        }
        catch (Exception $exception) {
          $model_has_datastream = '';
        }
        if ($model_has_datastream) {
          $successes[$dsid] = $dsid;
        }
        else {
          $failures[$dsid] = $dsid;
        }
      }
    }
    foreach ($check_dsids as $dsid) {
      if (array_key_exists($dsid, $good_dsids) === FALSE) {
        $failures[$dsid] = $dsid;
      }
    }
    islandora_object_validation_object_add_record($object, $check_dsids, $failures);
  }
}
