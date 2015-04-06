<?php
/**
 * @file
 * dosomething_api.strongarm.inc
 */

/**
 * Implements hook_strongarm().
 */
function dosomething_api_strongarm() {
  $export = array();

  $strongarm = new stdClass();
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'restful_file_upload';
  $strongarm->value = TRUE;
  $export['restful_file_upload'] = $strongarm;

  return $export;
}
