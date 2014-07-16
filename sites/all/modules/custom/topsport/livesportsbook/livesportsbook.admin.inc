<?php

/**
 * @file
 * Administrative functions and forms for livesportsbook.module
 */

function livesportsbook_settings_form() {
	$form = array();

	$form['topsport_WebOperationsAreAllowed'] = array(
    '#type' => 'select',
    '#title' => t('Global live betting status controller'),
    '#description' => t('This option let control access to live betting operations'),
    '#default_value' => variable_get('topsport_WebOperationsAreAllowed', TRUE ),
    '#options' => array(1 => t('Enabled'), 0 => t('Disabled')),
	);

	return system_settings_form($form);
}