<?php

/**
 * @file
 * Database functions for islandora_object_validation.
 */

/**
 * For adding a record to the islandora_object_validation_history table after
 * validation.
 *
 * @param stdObject $object
 *   This is the islandora object that was validated.
 * @param array $check_datastreams
 *   Array of datastream dsids as configured that were validated for the object.
 * @param array $missing_datastreams
 *   Array of datastream dsids that failed the validation test.
 * @param boolean $relationship_requirement_result
 *   The result of any relationship assertion for the object's models.
 */
function islandora_object_validation_object_add_record($object, $check_datastreams, $missing_datastreams, $relationship_requirement_result) {
  $validation_result = (count($missing_datastreams) > 0 || !$relationship_requirement_result) ? 0 : 1;
  // Convert these variables to a string for database storage.
  $check_datastreams_str = implode(",", array_keys($check_datastreams));
  $missing_datastreams_str = implode(",", $missing_datastreams);
  if ($check_datastreams_str == '' && $missing_datastreams_str == '') {
    $models = array();

    module_load_include('inc', 'islandora_object_validation', 'includes/utilities');
    module_load_include('inc', 'islandora_object_validation', 'includes/db');
    module_load_include('inc', 'islandora', 'includes/utilities');

    $object_models = $object->models;
    $validation_datastreams = variable_get('islandora_object_validation_selected_cmodels', array());
    $all_cmodels = islandora_get_content_models();
    $model_names = array();
    foreach ($object_models as $object_model) {
      $lc_name = str_replace("islandora:", "",  islandora_object_validation_normalize($object_model));
      // We only want to validate the overridden islandora models.
      if (strstr($object_model, 'fedora-system:') == '' && $lc_name) {
        $checks_added = FALSE;
        foreach ($validation_datastreams as $key => $validation_datastream) {
          $first_n_chars = substr($key, 0, strlen($lc_name) + 1);
          $checks_added |= ($first_n_chars == ($lc_name . '_'));
        }
        if (!$checks_added) {
          $models[$object_model] = $object_model;
          $model_names[$object_model] = islandora_object_validation_get_name_of_model_pid($all_cmodels, $object_model);
        }
      }
    }
    $check_datastreams_str = '[no datastreams configured for validation for ' . implode(",", $model_names) . ']';
  }
  $fields = array(
      'islandora_pid' => $object->id,
      'validation_result' => $validation_result,
      'check_datastreams' => $check_datastreams_str,
      'missing_datastreams' => $missing_datastreams_str,
      'relationship_requirement_result' => $relationship_requirement_result,
      'validation_time' => date("Y-m-d H:i:s", time()),
  );
  db_insert('islandora_object_validation_history')
      ->fields($fields)
      ->execute();
  $deleted = islandora_object_validation_remove_from_queue($object->id);
}

/**
 * Deletes a record from the validation queue for a given $pid.
 *
 * @param string $pid
 *   The pid of the islandora object.
 * @return stdObject
 *   A new DeleteQuery object.
 */
function islandora_object_validation_remove_from_queue($pid) {
  return db_delete('islandora_object_validation_queue')
    ->condition('islandora_pid', $pid)
    ->execute();
}

/**
 * This will queue an islandora object for validation.
 *
 * An object can only appear in the queue once, so if it exists already, the
 * insert code is skipped.
 *
 * @param stdObject $object
 *   This is the islandora object that is being queued for validation.
 * @return mixed
 *   A new InsertQuery object.
 */
function islandora_object_validation_add_queue_record($object) {
  $fields = array(
      'islandora_pid' => $object->id,
      'queued_timestamp' => date("Y-m-d H:i:s", time()),
  );
  if (!islandora_object_validation_queued_exists($object)) {
    // Now insert it.
    $inserted = db_insert('islandora_object_validation_queue')
      ->fields($fields)
      ->execute();
    return $inserted;
  }
  else {
    return TRUE;
  }
}


/**
 *
 * @param stdObject $object
 *   This is the islandora object that is being queued for validation.
 * @return boolean
 */
function islandora_object_validation_queued_exists($object) {
  $query = db_select('islandora_object_validation_queue', 'vq')
    ->fields('vq')
    ->condition('vq.islandora_pid', $object->id);
  $results = $query->execute();
  $row = $results->fetchAssoc();
  return (is_array($row) && array_key_exists('islandora_pid', $row) && $row['islandora_pid']);
}

/**
 *
 * @param type $how_many_per_chunk
 * @return type
 */
function islandora_object_validation_get_chunk_of_queued_items($how_many_per_chunk) {
  $drush_run = (function_exists('drush_get_option'));

  $query = db_select('islandora_object_validation_queue', 'vq')
    ->fields('vq')
    ->orderBy('vq.queued_timestamp', 'ASC')
    ->range(0,$how_many_per_chunk);
  $results = $query->execute();
  $rows = $results->fetchAll();
  $return_array = array();
  foreach ($rows as $row) {
    echo ($drush_run) ? print_r($row, true)."\n" : '';
    $return_array[$row->islandora_PID] = $row->islandora_PID;
  }
  return $return_array;
}

function islandora_object_validation_get_history_records($pid) {
  $query = db_select('islandora_object_validation_history', 'vh')
    ->fields('vh', array('validation_time', 'validation_result', 'missing_datastreams', 'check_datastreams', 'relationship_requirement_result'))
    ->condition('vh.islandora_pid', $pid)
    ->orderBy('vh.validation_time', 'ASC');
  $results = $query->execute();
  $rows = $results->fetchAll();
  $return_array = array();
  $good_span = '<span class="good">';
  $bad_span = '<span class="bad">';
  foreach ($rows as $row) {
    $row->validation_result = ($row->validation_result) ? $good_span . 'PASS</span>' : $bad_span . 'FAIL</span>';
    $row->missing_datastreams = ($row->missing_datastreams) ? $bad_span . $row->missing_datastreams . '</span>' : '';
    $row->relationship_requirement_result = ($row->relationship_requirement_result) ? $good_span . 'PASS</span>' : $bad_span . 'FAIL</span>';
    $return_array[] = (array)$row;
  }
  return $return_array;
}