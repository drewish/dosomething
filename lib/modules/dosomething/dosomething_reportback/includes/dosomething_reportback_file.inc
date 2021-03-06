<?php

/**
 * @file
 * Provides classes for the Reportback File Entity.
 */

/**
 * Our Reportback File entity class.
 */
class ReportbackFileEntity extends Entity {

  /**
   * Overrides to implement a custom default URI.
   */
  protected function defaultUri() {
    return array('path' => 'reportback/' . $this->rbid . '?fid=' . $this->identifier());
  }

  public function getImage($file_size = '300x300') {
    $image = dosomething_image_get_themed_image_by_fid($this->fid, $file_size);
    if (!$image) {
      return t("File removed.");
    }
    return $image;
  }
  public function getImageURL($file_size = '300x300') {
    $image = dosomething_image_get_themed_image_url_by_fid($this->fid, $file_size);
    if (!$image) {
      return t("File removed.");
    }
    return $image;
  }

  /**
   * Sets the Reportback File status and Reviewer details.
   *
   * @param array $values
   *   An associative array containing:
   *   - status (string).  Required.
   *   - source (string). The page or device the Review has been submitted from.
   *   - flagged_reason (string).
   *   - delete (integer).  If set to 1, delete the corresponding File entity.
   *
   */
  public function review($values) {
    global $user;
    if (!isset($values['status'])) {
      return FALSE;
    }
    $this->status = $values['status'];
    $this->reviewer = $user->uid;
    $this->reviewed = REQUEST_TIME;
    // Default source as the current URL path of page being viewed.
    $this->review_source = current_path();
    // If source was passed:
    if (isset($values['source'])) {
      // Store that instead.
      $this->review_source = $values['source'];
    }
    if ($this->status === 'flagged') {
      $reportback = reportback_load($this->rbid);
      $reason = NULL;
      if (isset($values['flagged_reason'])) {
        $reason = $values['flagged_reason'];
      }
      $reportback->flag($reason);
      if (!empty($values['delete'])) {
        $this->deleteFile();
      }
    }
    // Save the reviewed properties.
    return entity_save('reportback_file', $this);
  }

  /**
   * Deletes the File associated with this Reportback File.
   */
  public function deleteFile() {
    $file = file_load($this->fid);
    if ($file) {
      return file_delete($file);
    }
  }
}

/**
 * Our custom controller for the dosomething_reportback type.
 */
class ReportbackFileEntityController extends EntityAPIController {
  /**
   * Overrides buildContent() method.
   *
   * Adds Reportback properties into Reportback entity's render.
   */
  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {
    global $user;
    $build = parent::buildContent($entity, $view_mode, $langcode, $content);


    $build['image'] = array(
      '#markup' => $entity->getImage(),
    );

    $build['caption'] = array(
      '#prefix' => '<p><strong>',
      '#markup' => filter_xss($entity->caption),
      '#suffix' => '</strong></p>',
    );

    $file = file_load($entity->fid);
    $build['file_url'] = array(
      '#prefix' => '<p>',
      '#markup' => l(t("(View original upload)"), file_create_url($file->uri)),
      '#suffix' => '</p>',
    );

    if ($view_mode == 'full') {
      $reportback = reportback_load($entity->rbid);
      $author = user_load($reportback->uid);
      $files_count = count($reportback->fids);
      $total_files = t("@count files uploaded", array(
        '@count' => $files_count,
      ));

      $build['username'] = array(
        '#prefix' => '<p>',
        '#markup' => l($author->mail, 'user/' . $author->uid),
        '#suffix' => '</p>',
      );
      $build['quantity'] = array(
        '#prefix' => '<p>',
        '#markup' => '<strong>' . $reportback->quantity . '</strong> ' . $reportback->quantity_label,
        '#suffix' => '</p>',
      );
      if ($files_count > 1) {
        $build['num_files'] = array(
          '#prefix' => '<p>',
          '#markup' => l($total_files, 'reportback/'. $entity->rbid),
          '#suffix' => '</p>',
        );
      }

      $build['why'] = array(
        '#prefix' => '<p>',
        '#markup' => filter_xss($reportback->why_participated),
        '#suffix' => '</p>',
      );
    }

    if (!empty($entity->reviewed)) {
      $reviewer = user_load($entity->reviewer);
      $reason = NULL;
      if (!empty($reportback->flagged_reason)) {
        $reason = ' ' . t("as") . ' ' . $reportback->flagged_reason;
      }
      $build['reviewed'] = array(
        '#prefix' => '<p>',
        '#markup' => '<strong>' . ucfirst($entity->status) . '</strong> ' . format_date($entity->reviewed, 'short') . $reason,
        '#suffix' => '</p>',
      );
      if ($reviewer->uid > 0) {
        $build['reviewer'] = array(
          '#prefix' => '<p>',
          '#markup' => $reviewer->mail,
          '#suffix' => '</p>',
        );
      }
      $build['review_source'] = array(
        '#prefix' => '<p>',
        '#markup' => t("Source") . ': ' . $entity->review_source,
        '#suffix' => '</p>',
      );
    }
    return $build;
  }

  /**
   * Overrides save() method.
   *
   * Updates count variables for corresponding nodes and taxonomy terms.
   */
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    if (DOSOMETHING_REPORTBACK_LOG) {
      watchdog('dosomething_reportback_file', 'save:' . json_encode($entity));
    }
    $status = NULL;
    if (isset($entity->is_new)) {
      $op = 'insert';
      $status = 'pending';
    }
    parent::save($entity, $transaction);
    // Update the pending count for the node reportback was saved for.
    $reportback = reportback_load($entity->rbid);
    dosomething_reportback_reset_count('node', $reportback->nid, $status);
    if (module_exists('dosomething_campaign')) {
      if ($tid = dosomething_campaign_get_primary_cause_tid($reportback->nid)) {
        dosomething_reportback_reset_count('taxonomy_term', $tid, $status);
      }
    }
  }

  /**
   * Overrides delete() method.
   */
  public function delete($ids, DatabaseTransaction $transaction = NULL) {
    // Delete the related Files.
    foreach ($ids as $fid) {
      $rbf = reportback_file_load($fid);
      $rbf->deleteFile();
    }
    parent::delete($ids, $transaction);
  }
}
