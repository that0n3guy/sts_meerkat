<?php
// $Id$

/**
 * @file
 * Provides infrequently used functions for sts_meerkat.
 */

/**
 * Menu callback: display the menu block addition form.
 */
function sts_meerkat_add_block_form(&$form_state) {
  module_load_include('inc', 'block', 'block.admin');
  return block_admin_configure($form_state, 'sts_meerkat', NULL);
}

/**
 * Save the new menu block.
 */
function sts_meerkat_add_block_form_submit($form, &$form_state) {
  // Determine the delta of the new block.
  $block_ids = variable_get('sts_meerkat_ids', array());
  $delta = empty($block_ids) ? 1 : max($block_ids) + 1;

  // Save the new array of blocks IDs.
  $block_ids[] = $delta;
  variable_set('sts_meerkat_ids', $block_ids);

  // Save the block configuration.
  _sts_meerkat_block_save($delta, $form_state['values']);

  // Run the normal new block submission (borrowed from block_add_block_form_submit).
  foreach (list_themes() as $key => $theme) {
    if ($theme->status) {
      db_query("INSERT INTO {blocks} (visibility, pages, custom, title, module, theme, status, weight, delta, cache) VALUES(%d, '%s', %d, '%s', '%s', '%s', %d, %d, %d, %d)", $form_state['values']['visibility'], trim($form_state['values']['pages']), $form_state['values']['custom'], $form_state['values']['title'], $form_state['values']['module'], $theme->name, 0, 0, $delta, BLOCK_NO_CACHE);
    }
  }

  foreach (array_filter($form_state['values']['roles']) as $rid) {
    db_query("INSERT INTO {blocks_roles} (rid, module, delta) VALUES (%d, '%s', '%s')", $rid, $form_state['values']['module'], $delta);
  }

  drupal_set_message(t('The block has been created.'));
  cache_clear_all();

  $form_state['redirect'] = 'admin/build/block';
  return;
}

/**
 * Alters the block admin form to add delete links next to meerkat blocks.
 */
function _sts_meerkat_form_block_admin_display_form_alter(&$form, $form_state) {
  foreach (variable_get('sts_meerkat_ids', array()) AS $delta) {
    $form['sts_meerkat_' . $delta]['delete'] = array('#value' => l(t('delete'), 'admin/build/block/delete-meerkat-block/'. $delta));
  }
}

/**
 * Menu callback: confirm deletion of meerkat blocks.
 */
function sts_meerkat_delete(&$form_state, $delta = 0) {
  $title = _sts_meerkat_format_title(sts_meerkat_get_config($delta));
  $form['block_title'] = array('#type' => 'hidden', '#value' => $title);
  $form['delta'] = array('#type' => 'hidden', '#value' => $delta);

  return confirm_form($form, t('Are you sure you want to delete the "%name" block?', array('%name' => $title)), 'admin/build/block', NULL, t('Delete'), t('Cancel'));
}

/**
 * Deletion of meerkat blocks.
 */
function sts_meerkat_delete_submit($form, &$form_state) {
  // Remove the meerkat block configuration variables.
  $delta = $form_state['values']['delta'];
  $block_ids = variable_get("sts_meerkat_ids", array());
  unset($block_ids[array_search($delta, $block_ids)]);
  sort($block_ids);
  variable_set("sts_meerkat_ids", $block_ids);
  variable_del("sts_meerkat_{$delta}_admin_title");
  variable_del("sts_meerkat_{$delta}_block_content");
  variable_del("sts_meerkat_{$delta}_meerkat_settings");
  variable_del("sts_meerkat_{$delta}_format");
  variable_del("sts_meerkat_{$delta}_title");
  variable_del("sts_meerkat_{$delta}_css");
  variable_del("sts_meerkat_{$delta}_session_close");
  variable_del("sts_meerkat_{$delta}_session_selector");


  /* Maybe later
      variable_del("sts_meerkat_{$delta}_background");
    variable_del("sts_meerkat_{$delta}_opacity");
    variable_del("sts_meerkat_{$delta}_height");
    variable_del("sts_meerkat_{$delta}_width");
    variable_del("sts_meerkat_{$delta}_position");
    variable_del("sts_meerkat_{$delta}_close");
    variable_del("sts_meerkat_{$delta}_dontShowAgain");
    variable_del("sts_meerkat_{$delta}_cookieExpires");
    variable_del("sts_meerkat_{$delta}_removeCookie");
    variable_del("sts_meerkat_{$delta}_animationIn");
    variable_del("sts_meerkat_{$delta}_animationOut");
    variable_del("sts_meerkat_{$delta}_animationSpeed");
    variable_del("sts_meerkat_{$delta}_delay");
    variable_del("sts_meerkat_{$delta}_timer");
    variable_del("sts_meerkat_{$delta}_dontShowAgainAuto");
    variable_del("sts_meerkat_{$delta}_onMeerkatShow");
    variable_del("sts_meerkat_{$delta}_easingIn");
    variable_del("sts_meerkat_{$delta}_easingOut");
  */

  db_query("DELETE FROM {blocks} WHERE module = 'sts_meerkat' AND delta = %d", $delta);
  db_query("DELETE FROM {blocks_roles} WHERE module = 'sts_meerkat' AND delta = %d", $delta);
  drupal_set_message(t('The "%name" block has been removed.', array('%name' => $form_state['values']['block_title'])));
  cache_clear_all();
  $form_state['redirect'] = 'admin/build/block';
  return;
}

/**
 * Returns the 'list' $op info for hook_block().
 */
function _sts_meerkat_block_list() {
  $blocks = array();
  foreach (variable_get("sts_meerkat_ids", array()) AS $delta) {
    $blocks[$delta]['info'] = _sts_meerkat_format_title(sts_meerkat_get_config($delta));
    //$blocks[$delta]['cache'] = BLOCK_NO_CACHE;
  }
  return $blocks;
}

/**     //////////////////////todo   redo this


 * Return the title of the block.
 *
 * @param $config
 *   array The configuration of the menu block.
 * @return
 *   string The title of the block.
 */
function _sts_meerkat_format_title($config) {
//  $title = db_query("SELECT title FROM {blocks} WHERE delta = %s", $config);
  return "STScode Meerkat - " . $config['admin_title'];
}


/**
 * Returns the 'configure' $op info for hook_block().
 */
function _sts_meerkat_block_configure($delta) {
  // Create a pseudo form state.
  $form_state = array('values' => sts_meerkat_get_config($delta));
  return sts_meerkat_configure_form($form_state);
}


/**
 * Returns the configuration form for a menu tree.
 *
 * @param $form_state
 *   array An associated array of configuration options should be present in the
 *   'values' key. If none are given, default configuration is assumed.
 * @return
 *   array The form in Form API format.
 */
function sts_meerkat_configure_form(&$form_state) {
  $config = array();
  if ($path = libraries_get_path('jquery.meerkat')) {
    // great... your library is set in the right place
  }
  else {
    drupal_set_message('You have not installed the jquery.meerkat library yet.  Please do so.', $type = 'warning', $repeat = TRUE);
  }

  // Get the config from the form state.
  if (!empty($form_state['values'])) {
    $config = $form_state['values'];
  }

  // Merge in the default configuration.
  $config += sts_meerkat_get_config();

  $form['admin_title'] = array(
    '#type' => 'textfield',
    '#default_value' => $config['admin_title'],
    '#title' => t('Administrative title'),
    '#description' => t('This title will be used administratively to identify this block. If blank, the regular title will be used.'),
    '#required' => TRUE
  );

  $form['block_content']['text'] = array(
    '#type' => 'textarea',
    '#title' => t('Meerkat Content'),
    '#description' => t('The content that will go into the actual meerkat popup'),
    '#default_value' => $config['block_content'],
    '#required' => TRUE
  );
  $form['block_content']['format'] = filter_form($config['format'], NULL, array('format'));

  ctools_include('dependent');
  $form['session'] = array(
    '#type' => 'fieldset', 
    '#title' => t('Close for the Session settings'), 
    '#collapsible' => TRUE, 
    '#collapsed' => FALSE,
  );
  $form['session']['meerkat_session_close'] = array(
    '#type' => 'select',
    '#title' => t('When the "hide" button is pushed, keep the Meerkat closed until the next drupal session?'),
    '#default_value' => $config['session_close'],
    '#options' => array(
      '0' => t('No'),
      '1' => t('Yes'),
    ),
  );
  $form['session']['meerkat_session_selector'] = array(
    '#type' => 'textfield',
    '#default_value' => $config['session_selector'],
    '#title' => t('Session CSS Selector'),
    '#description' => t('The css selector to be used to set the meerkat popup from showing, only for this drupal session.  Commonly used with "hide" instead of using the "dontshow" meerkat option.'),
    '#required' => TRUE,
   // 2. This ensures that the element will be processed by ctools dependent
   '#process' => array('ctools_dependent_process'),
   // 3. This instructs ctools_dependent which other elements this
   //     should depend on. It has the following format:
   //     #dependency => array('id-of-element-to-depend-on' => array('any_value', 'will', 'trigger', 'me'))
   '#dependency' => array('edit-meerkat-session-close' => array('1')),  
);

  $form['meerkat_settings'] = array(
    '#type' => 'textarea',
    '#title' => t('Meerkat settings'),
    '#default_value' => $config['meerkat_settings'],
    '#required' => TRUE
  );
  $form['meerkat_css'] = array(
    '#type' => 'textarea',
    '#title' => t('Meerkat CSS'),
    '#default_value' => $config['css'],
    '#required' => TRUE
  );

  return $form;
}







/**
 * Validates the parent element of the block configuration form.
 */
/*
function menu_block_configure_form_parent_validate($element, &$form_state) {
  // The "Menu block" config has the "parent" value split into "menu_name" and
  // "parent_mlid" values. The value of "parent" stored in the database/config array is either FALSE
  list($form_state['values']['menu_name'], $form_state['values']['parent_mlid']) = explode(':', $form_state['values']['parent']);
}
*/

/**
 * Validates the follow element of the block configuration form.
 */
/*
function menu_block_configure_form_follow_validate($element, &$form_state) {
  // The value of "follow" stored in the database/config array is either FALSE
  // or the value of the "follow_parent" form element.
  if ($form_state['values']['follow'] && !empty($form_state['values']['follow_parent'])) {
    $form_state['values']['follow'] = $form_state['values']['follow_parent'];
  }
}
*/






/**
 * Returns the 'save' $op info for hook_block().
 */
function _sts_meerkat_block_save($delta, $edit) {
  variable_set("sts_meerkat_{$delta}_admin_title", $edit['admin_title']);
  variable_set("sts_meerkat_{$delta}_block_content",    $edit['text']);
  variable_set("sts_meerkat_{$delta}_meerkat_settings",    $edit['meerkat_settings']);
  variable_set("sts_meerkat_{$delta}_format",    $edit['format']);
  variable_set("sts_meerkat_{$delta}_title",    $edit['title']);
  variable_set("sts_meerkat_{$delta}_css",    $edit['meerkat_css']);
  variable_set("sts_meerkat_{$delta}_session_close",    $edit['meerkat_session_close']);
  variable_set("sts_meerkat_{$delta}_session_selector",    $edit['meerkat_session_selector']);
  
  /* Maybe later
    variable_set("sts_meerkat_{$delta}_background",    $config['background']);
    variable_set("sts_meerkat_{$delta}_opacity",    $config['opacity']);
    variable_set("sts_meerkat_{$delta}_height",    $config['height']);
    variable_set("sts_meerkat_{$delta}_width",    $config['width']);
    variable_set("sts_meerkat_{$delta}_position",    $config['position']);
    variable_set("sts_meerkat_{$delta}_close",    $config['close']);
    variable_set("sts_meerkat_{$delta}_dontShowAgain",    $config['dontShowAgain']);
    variable_set("sts_meerkat_{$delta}_cookieExpires",    $config['cookieExpires']);
    variable_set("sts_meerkat_{$delta}_removeCookie",    $config['removeCookie']);
    variable_set("sts_meerkat_{$delta}_animationIn",    $config['animationIn']);
    variable_set("sts_meerkat_{$delta}_animationOut",    $config['animationOut']);
    variable_set("sts_meerkat_{$delta}_animationSpeed",    $config['animationSpeed']);
    variable_set("sts_meerkat_{$delta}_delay",    $config['delay']);
    variable_set("sts_meerkat_{$delta}_timer",    $config['timer']);
    variable_set("sts_meerkat_{$delta}_dontShowAgainAuto",    $config['dontShowAgainAuto']);
    variable_set("sts_meerkat_{$delta}_onMeerkatShow",    $config['onMeerkatShow']);
    variable_set("sts_meerkat_{$delta}_easingIn",    $config['easingIn']);
    variable_set("sts_meerkat_{$delta}_easingOut",    $config['easingOut']);
  */
}


/**
 * Menu block admin settings form.
 *
 * @return
 *   The settings form used by Menu block.
 */
function sts_meerkat_admin_settings_form($form_state) {
  // Load stored configuration.
  $menu_order = variable_get('menu_block_menu_order', array('primary-links' => '', 'secondary-links' => ''));

///////////////////put in location of jquery library.
if ($path = libraries_get_path(jquery.meerkat)) {
  // great... your library is set in the right place

}
else {
  //you need to put the library in the right place.
}


// instructions on how to add a block... links

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Form submission handler.
 */
function sts_meerkat_admin_settings_form_submit($form, &$form_state) {
//nothing to set
//  variable_set('menu_block_menu_order', $menu_order);
}
