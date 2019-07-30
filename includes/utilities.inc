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
      'title' => t('link opens in separate tab'),
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

function islandora_object_validation_get_pids_solr_query($solr_query, $limit = 0) {
  return implode('
', islandora_object_validation_get_pids($solr_query, $limit));
}

function islandora_object_validation_get_pids_collection($collection, $limit = 0) {
  $query = format_string('!field:!value', array(
    '!field' => 'RELS_EXT_isMemberOfCollection_uri_ms',
    '!value' => str_replace(array("/", ":", "-", "+"), array("\/", "\:", "\-", "\+"), $collection),
   ));
  return implode('
', islandora_object_validation_get_pids($query, $limit));
}

function islandora_object_validation_get_pids_model($model, $limit = 0) {
  $query = format_string('!field:!value', array(
    '!field' => 'RELS_EXT_hasModel_uri_ms',
    '!value' => str_replace(array("/", ":", "-", "+"), array("\/", "\:", "\-", "\+"), $model),
   ));
  return implode('
', islandora_object_validation_get_pids($query, $limit));
}

function islandora_object_validation_get_pids($query, $limit = 0) {
  module_load_include('inc', 'islandora_solr', 'includes/utilities');
  if (!$limit) {
    $limit = variable_get('islandora_object_validation_solr_limit', 10000);
  }
  $query_processor = new IslandoraSolrQueryProcessor();
  $ret_arr = array();
  // run two queries --
  // 1) to get the datastreams available on this object
  $query_processor->solrQuery = $query;
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = $limit;
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

function islandora_object_validation_normalize($in) {
  return str_replace("_", "", strtolower($in));
}

/**
 * Performs the actual validation of an islandora object using the configuration.
 *
 * @param stdObject $object
 *   The islandora object being validated.
 */
function islandora_object_validation_validate($object) {
  // Perform the validation check on this object only if this is configured to happen.
  if (is_object($object)) {
    module_load_include('inc', 'islandora_object_validation', 'includes/db');
    // Get the $object's model and check to see if there are any
    $object_models = $object->models;
    $check_dsids = array();
    $relationship_requirement_result = TRUE;
    $validation_datastreams = variable_get('islandora_object_validation_selected_cmodels', array());
    $model_relationships = variable_get('islandora_object_validation_model_relationships', array());
    foreach ($object_models as $object_model) {
      $lc_name = str_replace("islandora:", "",  islandora_object_validation_normalize($object_model));
      // We only want to validate the overridden islandora models.
      if (strstr($object_model, 'fedora-system:') == '' && $lc_name) {
        // Check for any relationship validation requirement.
        $relationship_requirement_result &= islandora_object_validation_validate_relationship($object, $model_relationships[$lc_name]);
        foreach ($validation_datastreams as $key => $validation_datastream) {
          $first_n_chars = substr($key, 0, strlen($lc_name) + 1);
          if ($first_n_chars == ($lc_name . '_')) {
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
        if ($model_has_datastream && $model_has_datastream->size > 0) {
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
    islandora_object_validation_object_add_record($object, $check_dsids, $failures, $relationship_requirement_result);
  }
}

function islandora_object_validation_validate_relationship($object, $model_config) {
  // Require that there are values in all of the fields
  if (array_key_exists('relationship', $model_config) &&
    $model_config['relationship'] && array_key_exists('howmany', $model_config) &&
    $model_config['howmany'] && array_key_exists('related_to_model', $model_config) &&
    $model_config['related_to_model']) {
    $rels_ext = $object->relationships;
    $object_relationships_in_question = $rels_ext->get(ISLANDORA_RELS_EXT_URI, $model_config['relationship']);
    // There should be at least one array element returned with this relationship
    $matches = 0;
    // at least one relationship exists for the model with the predicate
    foreach ($object_relationships_in_question as $object_relationship) {
      // Reference the underlying object that this relationship points to.
      if (array_key_exists('object', $object_relationship) &&
          array_key_exists('value', $object_relationship['object'])) {
        $rel_obj_pid = $object_relationship['object']['value'];
        $related_object = islandora_object_load($rel_obj_pid);
        if (is_object($related_object)) {
          $related_object_models = $related_object->models;
          foreach ($related_object_models as $related_object_model) {
            // Increment $matches if the model name matches the related
            // object's model name.
            if (strstr($related_object_model, 'fedora-system:') == '' &&
                $related_object_model == $model_config['related_to_model']) {
              $matches++;
            }
          }
        }
      }
    }
    if ($model_config['howmany'] == 1 && $matches > 1) {
      // Can fail this here - config in this case wanted EXACTLY 1 relationship
      // but more than 1 was found.
      drupal_set_message(t('Object validation required a relationship to EXACTLY 1 ' .
          '`@related_to_model` object but @matches were found!', array(
              '@related_to_model' => $model_config['related_to_model'],
              '@matches' => $matches,
          )), 'error');
      return FALSE;
    }
    elseif ($matches < 1) {
      drupal_set_message(t('Object validation required a relationship to EXACTLY 1 ' .
          '`@related_to_model` object but no matches were found!', array(
              '@related_to_model' => $model_config['related_to_model'],
          )), 'error');
      return FALSE;
    }
    else {
      return TRUE;
    }
  }
  else {
    return TRUE;
  }
}

function islandora_object_validation_get_select_box($model_relationships, $which, $use_name, $object_pid = '', $all_models = array()) {
  // The first part of the control is the same for all modes of $which
  $open_select = '<select class="m_select ' . $which . '" id="' . $use_name . 
      '_' . $which . '" name="' . $use_name . '_' . $which . '">';
  switch ($which) {
    case 'howmany':
      $return_markup = $open_select .
        '<option value="">None</option>' .
        '<option value="1">Exactly 1</option>' .
        '<option value="2">At least 1</option>' .
        '</select>';
      break;
    case 'relationship':
      // These can just be hard-coded and would require a code update in order to
      // add support for any other relationships.
      $relationships = array('isMemberOf', 'isPageOf', 'isConstituentOf');
      $options = '<option value="">None</option>';
      foreach ($relationships as $relationship) {
        $selected = (array_key_exists($use_name, $model_relationships) && array_key_exists('relationship', $model_relationships[$use_name]) && $model_relationships[$use_name]['relationship'] == $relationship) ?
            ' selected' : '';
        $options .= '<option value="' . $relationship . '"' . $selected . '>' .
            $relationship . '</option>';
      }
      $return_markup = $open_select .
          $options .
        '</select>';
      break;
    case 'related_to_model':
      $options = '<option value="">None</option>';
      foreach ($all_models as $pid => $label) {
        // Object models are never related to the same model, so only add the
        // ones that are not $object_pid value.
        if ($pid <> $object_pid) {
          $options .= '<option value="' . $pid . '">' . $label . '</option>';
        }
      }
      $return_markup = $open_select .
          $options .
        '</select>';
      break;

    default:
      break;
  }
  return $return_markup;
}

function islandora_object_validation_get_name_of_model_pid($all_cmodels, $object_model) {
  $model_match = '';
  foreach ($all_cmodels as $pid_label_arr) {
    if (!$model_match) {
      if (array_key_exists('label', $pid_label_arr) && array_key_exists('pid', $pid_label_arr) && $pid_label_arr['pid'] == $object_model) {
        $model_match = $pid_label_arr['label'];
      }
    }
  }
  return $model_match;
}

function islandora_object_validation_get_validations_table($cmodels) {
  $dsids = array();
  $reqs = array();
  $already_chosen = variable_get('islandora_object_validation_selected_cmodels', array());
  $model_relationships = variable_get('islandora_object_validation_model_relationships', array());

  $all_models = array();
  foreach($cmodels as $key => $pid_label) {
    $pid = $pid_label['pid'];
    $name = $pid_label['label'];
    $all_models[$pid] = $name;
    $req = islandora_object_validation_object_model_datastream_requirements($pid);
    $reqs[$pid] = $req;

    $all = (array_key_exists('required', $req) || array_key_exists('optional', $req)) ?
        array_merge($req['required'], ((array_key_exists('optional', $req) && is_array($req['optional'])) ? $req['optional'] : array())) : array();
    foreach ($all as $dsid => $ds_req) {
      $dsids[$dsid] = $dsid;
    }
  }
  ksort($dsids);
  $rows = array();
  $dsid_models = array();
  foreach($cmodels as $key => $pid_label) {
    $pid = $pid_label['pid'];
    $name = $pid_label['label'];
    $use_name = str_replace('islandora:', "", islandora_object_validation_normalize($pid));
    $req = $reqs[$pid];
    $row = array();
    $select_box_howmany = islandora_object_validation_get_select_box($model_relationships, 'howmany', $use_name);
    $select_box_othermodels = islandora_object_validation_get_select_box($model_relationships, 'related_to_model', $use_name, $pid, $all_models);
    $select_box_relationships = islandora_object_validation_get_select_box($model_relationships, 'relationship', $use_name);
    // Model
    $row[] = '<div class="model_label" title="' . $name . '"><label>' . $name . '</label>' .
        '<span class="pid">' . l($pid, 'islandora/object/'. $pid) . '</span><br>' .
        '<span title="Relationship requirement" class="fineprint">Requires a ' .
        $select_box_relationships . ' relationship to ' .
        $select_box_howmany . '<br>' .
        $select_box_othermodels . ' ' .
        '</div>';
    foreach ($dsids as $dsid) {
      $cbx_name = $use_name . '_' . $dsid;
      $cbx_val = array_key_exists($cbx_name, $already_chosen) ? ' checked' : '';
      $optional = (array_key_exists('optional', $req) && array_key_exists($dsid, $req['optional']));
      $req_class = (array_key_exists('required', $req) && array_key_exists($dsid, $req['required'])) ? ' required' : '';
      if ($req_class) {
        $dsid_models[$dsid][$pid] = $name;
      }
      $cell_title = $name . ' &rarr; ' . $dsid . " " . ($optional ? 'optional' : ($req_class ? 'required' : 'N/A'));
      $checkbox = (($req_class ||
          $optional)  ?
          '<input type="checkbox"' . $cbx_val . ' name="' . $cbx_name . '" id="' . $cbx_name . '">' :
          // even though some datastreams are N/A, they may still need to be
          // added to objects like in the case of aggregated PDF and consolidated
          // OCR for paged content.
          '<input type="checkbox"' . $cbx_val . ' name="' . $cbx_name . '" id="' . $cbx_name . '">');
//          '<input type="checkbox"' . $cbx_val . ' disabled name="' . $cbx_name . '" id="' . $cbx_name . '">');
      $row[] = '<div class="checkbox_cell centered' . $req_class . ' fineprint" ' .
          'id="cell_' . $cbx_name . '" title="' . $cell_title . '">' .
          $checkbox . '<br>' . (array_key_exists('required', $req) && array_key_exists($dsid, $req['required']) ? '&nbsp;' :
          (array_key_exists('optional', $req) && array_key_exists($dsid, $req['optional']) ? '' : '<span class="item_na">N/A</span>')) .
          '</div>';
    }
    $rows[] = $row;
  }
  $headers = array_keys(array_merge(array('Model' => 'Model'), $dsids));
  $table_markup = theme('table', array('header' => $headers, 'rows' => $rows));
  // add which models require usage of each datastream - based only on the
  // islandora_get_datastreams_requirements_from_content_model core function.
  // We shouldn't need to perform string replacements on a chunk of HTML, but
  // there doesn't seem to be a way to populate the table heading cells'
  // title attributes -- I searched for 30 minutes but give up.
  foreach ($dsids as $dsid) {
    if (array_key_exists($dsid, $dsid_models) && is_array($dsid_models[$dsid]) && count($dsid_models[$dsid]) > 0) {
      $search = '>' . $dsid . '</th>';
      $replace = '><span title="' . htmlentities(t('Required by "') . implode('", "', $dsid_models[$dsid]) . '"') . '">' . $dsid . '</span></th>';
      $table_markup = str_replace($search, $replace, $table_markup);
    }
  }
  return $table_markup;
}


function islandora_object_validation_get_history_table($pid) {
  module_load_include('inc', 'islandora_object_validation', 'includes/db');
  $history_records = islandora_object_validation_get_history_records($pid);

  $headers = array(
      'validation_time' => 'Validation Date',
      'validation_result' => 'Valid?',
      'missing_datastreams' => 'Failure datastreams',
      'check_datastreams' => 'Checked datastreams',
      'relationship_requirement_result' => 'Valid Relationship?',
      );
  return theme('table', array('header' => $headers, 'rows' => $history_records));
}
