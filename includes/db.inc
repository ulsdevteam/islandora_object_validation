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
}

function islandora_object_validation_add_queue_record($object) {
  $fields = array(
      'islandora_pid' => $object->id,
      'queued_timestamp' => date("Y-m-d H:i:s", time()),
  );
  db_insert('islandora_object_validation_queue')
      ->fields($fields)
      ->execute();
}
