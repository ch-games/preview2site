<?php

function account_admin_activation(){
  $header = array(
    'username' => array('data' => t('Username')),
    'firstname' => array('data' => t('Vardas')),
    'surname' => array('data' => t('Pavardė')),
    'email' => array('data' => t('Email')),
    'member_for' => array('data' => t('Member for')),
    'access' => array('data' => t('Last access')),
    'operations' => array('data' => t('Operations')),
  );
  $destination = drupal_get_destination();
  $pendingUsers = account_get_pending_users();
  $rows = array();
  foreach($pendingUsers as $pendingUser){
    $pendingUser->data = unserialize($pendingUser->data);
    $rows[] = array(
      'username' => $pendingUser->name,
      'firstname' => $pendingUser->data['firstname'],
      'surname' => $pendingUser->data['surname'],
      'email' => $pendingUser->mail,
      'member_for' => format_interval(REQUEST_TIME - $pendingUser->created),
      'access' =>  $pendingUser->access ? t('@time ago', array('@time' => format_interval(REQUEST_TIME - $pendingUser->access))) : t('never'),
      'operations' => array('data' => array('#type' => 'link', '#title' => t('edit'), '#href' => "admin/people/activation/$pendingUser->uid", '#options' => array('query' => $destination))),
    );
  }
  $empty = t('Laukančių aktyvacijos nėra');
  return theme('table', array('header' => $header, 'rows' => $rows, 'empty' => $empty));
}


function account_get_pending_users(){
  return db_query('select * from {users} WHERE "data" LIKE \'%s:14:"agreement_step";i:4;%\' ORDER BY access DESC')->fetchAll();
}


function account_get_rejected_users(){
  return db_query('select * from {users} WHERE "data" LIKE \'%s:14:"agreement_step";d:3%\'')->fetchAll();
}


function account_admin_rejected(){
  $header = array(
    'username' => array('data' => t('Username')),
    'firstname' => array('data' => t('Vardas')),
    'surname' => array('data' => t('Pavardė')),
    'email' => array('data' => t('Email')),
    'comment' => array('data' => t('Komentaras')),
    'member_for' => array('data' => t('Member for')),
    'access' => array('data' => t('Last access')),
  );
  $destination = drupal_get_destination();
  $pendingUsers = account_get_rejected_users();
  $rows = array();
  foreach($pendingUsers as $pendingUser){
    $pendingUser->data = unserialize($pendingUser->data);
    $rows[] = array(
      'username' => l($pendingUser->name, 'user/'.$pendingUser->uid.'/edit'),
      'firstname' => $pendingUser->data['firstname'],
      'surname' => $pendingUser->data['surname'],
      'email' => $pendingUser->mail,
      'comment' => !empty($pendingUser->data['agreement_reject_comment']) ? $pendingUser->data['agreement_reject_comment'] : '',
      'member_for' => format_interval(REQUEST_TIME - $pendingUser->created),
      'access' =>  $pendingUser->access ? t('@time ago', array('@time' => format_interval(REQUEST_TIME - $pendingUser->access))) : t('never'),
    );
  }
  $empty = t('Atmestų vartotojų nėra');
  $build['user_table'] = array('#theme' => 'table', '#header' => $header, '#rows' => $rows, '#empty' => $empty);
  return $build;
}

function account_admin_DepositReport($UserID){
  $header = array(
    'username' => array('data' => t('Username')),
    'firstname' => array('data' => t('Vardas')),
    'surname' => array('data' => t('Pavardė')),
    'email' => array('data' => t('Email')),
    'comment' => array('data' => t('Komentaras')),
    'member_for' => array('data' => t('Member for')),
    'access' => array('data' => t('Last access')),
  );
  $pendingUsers = account_get_DepositReport($UserID);
  $rows = array();
  foreach($pendingUsers as $pendingUser){
    $pendingUser->data = unserialize($pendingUser->data);
    $rows[] = array(
      'username' => l($pendingUser->name, 'user/'.$pendingUser->uid.'/edit'),
      'firstname' => $pendingUser->data['firstname'],
      'surname' => $pendingUser->data['surname'],
      'email' => $pendingUser->mail,
      'comment' => !empty($pendingUser->data['agreement_reject_comment']) ? $pendingUser->data['agreement_reject_comment'] : '',
      'member_for' => format_interval(REQUEST_TIME - $pendingUser->created),
      'access' =>  $pendingUser->access ? t('@time ago', array('@time' => format_interval(REQUEST_TIME - $pendingUser->access))) : t('never'),
    );
  }
  $empty = t('Atmestų vartotojų nėra');
  $build['user_table'] = array('#theme' => 'table', '#header' => $header, '#rows' => $rows, '#empty' => $empty);
  return $build;
}

function account_get_DepositReport($UserID){
  return db_query('select * from {users_orders} WHERE uid = #uid ORDER BY timestamp DESC', array('#uid' => $UserID))->fetchAll();
}