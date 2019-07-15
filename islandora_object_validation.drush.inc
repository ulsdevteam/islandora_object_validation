<?php

/**
 * @file
 * Drush hooks.
 */

/**
 * Implements hook_drush_command().
 */
function islandora_object_validation_drush_command() {
  $items = array();

  $items['islandora_object_validation_validate_chunk_of_objects'] = array(
    'aliases' => array('iovvo'),
    'callback' => 'islandora_object_validation_validate_chunk_of_objects',  // Callback function
    'description' => 'Perform validation on a set of islandora objects from the module\'s queue table.',
    'drupal dependencies' => array(),
    'examples' => array(
      'drush -v --user=admin --uri=http://gamera.library.pitt.edu islandora_object_validation_validate_chunk_of_objects --howmany=500',
    ),
    'options' => array(

    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  return $items;
}

/**
 * Implements hook_islandora_batch_ingest().
 *
 * Queries for a set of objects from the queue table - and processes them for
 * validation.
 */
function islandora_object_validation_validate_chunk_of_objects() {
  module_load_include('inc', 'islandora_object_validation', 'includes/db');
  module_load_include('inc', 'islandora_object_validation', 'includes/utilities');

//  module_load_include('module', 'islandora_object_validation', 'islandora_object_validation');
  // How many objects should be processed?
  //  -- check for a drush parameter for how many, if that is not set use the
  // value from islandora_object_validation_cron_limit variable.
  $howmany = drush_get_option('howmany', NULL);
  if (is_null($howmany)) {
    $howmany = variable_get('islandora_object_validation_cron_limit', 4000);
  }
  $queued_objects = islandora_object_validation_get_chunk_of_queued_items($howmany);
  echo "islandora_object_validation module Drush command validate_chunk_of_objects\n" .
      "How many = " . number_format($howmany) . "\n" .
      "Items returned = " . number_format(count($queued_objects)) .
      "\n------------\n";
  foreach ($queued_objects as $pid) {
    $islandora_object = islandora_object_load($pid);
    if (is_object($islandora_object)) {
      islandora_object_validation_validate($islandora_object);
      echo $pid . " validated\n";
    }
  }
  echo "done\n\n";
}