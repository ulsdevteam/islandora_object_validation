<?php

/**
 * @file
 * Database functions for islandora_object_validation.
 */

function islandora_object_validation_object_add_record($object, $check_dsids, $failures) {
  $validation_result = (count($failures) > 0) ? 0 : 1;
  $check_datastreams = implode(",", $check_dsids);
  $missing_datastreams = implode(",", $failures);
  $fields = array(
      'islandora_pid' => $object->id,
      'validation_result' => $validation_result,
      'check_datastreams' => $check_datastreams,
      'missing_datastreams' => $missing_datastreams,
      'validation_time' => date("Y-m-d H:i:s", time()),
  );
  db_insert('islandora_object_validation_history')
      ->fields($fields)
      ->execute();
  $deleted = db_delete('islandora_object_validation_queue')
    ->condition('islandora_pid', $object->id)
    ->execute();
}

function islandora_object_validation_add_queue_record($object) {
  $fields = array(
      'islandora_pid' => $object->id,
      'queued_timestamp' => date("Y-m-d H:i:s", time()),
  );
  // Delete this queue record
  $deleted = db_delete('islandora_object_validation_queue')
    ->condition('islandora_pid', $object->id)
    ->execute();
  $inserted = db_insert('islandora_object_validation_queue')
    ->fields($fields)
    ->execute();
  return $inserted;
}

function islandora_object_validation_get_chunk_of_queued_items($how_many_per_chunk) {
  $query = db_select('islandora_object_validation_queue', 'vq')
    ->fields('vq')
    ->orderBy('vq.queued_timestamp', 'ASC')
    ->range(0,$how_many_per_chunk);
  $results = $query->execute();
  $rows = $results->fetchAll();
  $return_array = array();
  foreach ($rows as $row) {
    echo print_r($row, true)."\n";
    $return_array[$row->islandora_PID] = $row->islandora_PID;
  }
  return $return_array;
}