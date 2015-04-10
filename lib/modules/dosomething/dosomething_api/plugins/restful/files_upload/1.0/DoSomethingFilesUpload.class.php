<?php

/**
 * @file
 * Contains DoSomethingFilesUpload.
 */

class DoSomethingFilesUpload extends \RestfulFilesUpload {

  /**
   * Overrides \RestfulFilesUpload::createEntity().
   *
   * Does not make files permanent and does not add a usage.
   */
  public function createEntity() {
    if (!$_FILES) {
      throw new \RestfulBadRequestException('No files sent with the request.');
    }

    $ids = array();

    foreach ($_FILES as $file_info) {
      // Populate the $_FILES the way file_save_upload() expects.
      $name = $file_info['name'];
      foreach ($file_info as $key => $value) {
        $_FILES['files'][$key][$name] = $value;
      }

      $file = $this->fileSaveUpload($name);

# Commenting this out because it's totally against the normal Drupal upload flow.
      // // Change the file status from temporary to permanent.
      // $file->status = FILE_STATUS_PERMANENT;
      // file_save($file);

      // // Required to be able to reference this file.
      // file_usage_add($file, 'restful', 'files', $file->fid);

      $ids[] = $file->fid;
    }

    $return = array();
    foreach ($ids as $id) {
      $return[] = $this->viewEntity($id);
    }

    return $return;
  }

  /**
   * Overrides RestfulEntityBase::access().
   *
   * If "File entity" module exists, determine access by its provided permissions
   * otherwise, check if variable is set to allow anonymous users to upload.
   * Defaults to authenticated user.
   */
  public function access() {
    // The getAccount method may return a RestfulUnauthorizedException when an
    // authenticated user cannot be found. Since this is called from the access
    // callback, not from the page callback we need to catch the exception.
    try {
      $account = $this->getAccount();
# TODO Need to figure out permissions.
return true;
    }
    catch (\RestfulUnauthorizedException $e) {
      // If a user is not found then load the anonymous user to check
      // permissions.
      $account = drupal_anonymous_user();
    }
return false;

    if (module_exists('file_entity')) {
      return user_access('bypass file access', $account) || user_access('create files', $account);
    }

    return (variable_get('restful_file_upload_allow_anonymous_user', FALSE) || $account->uid) && parent::access();
  }
}
