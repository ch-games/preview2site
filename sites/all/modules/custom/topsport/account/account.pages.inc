<?php

function account_download_userCard($uid, $timestamp, $hashed_pass){
	$timeout = 86400;
	$current = REQUEST_TIME;
	$users = user_load_multiple(array($uid));
	if ($timestamp <= $current && $account = reset($users)) {
		if ($account->login && $current - $timestamp > $timeout) {
			// Timeout
			drupal_set_message (t('Nuoroda nebegalioja'), 'warning');
		}elseif ($account->uid && $timestamp >= $account->login && $timestamp <= $current && $hashed_pass == user_pass_rehash($account->pass, $timestamp, $account->login)) {
			ob_end_clean();
			db_update('users')->fields(array('login' => $current))->condition('uid', $uid)->execute();
			account_generatePinCardPDF($account, true);
		}else {
			drupal_set_message (t('Nuoroda nebegalioja'), 'warning');
		}
		drupal_goto();
	}else{
		drupal_access_denied();
	}
}

/**
 * Menu callback; process one time login link and redirects to the user page on success.
 */
function account_pass_reset($form, &$form_state, $uid, $timestamp, $hashed_pass, $action = NULL) {
	global $user;
	drupal_set_title(t('Mano erdvė, naujo slaptažodžio nustatymas'));
	// When processing the one-time login link, we have to make sure that a user
	// isn't already logged in.
	if ($user->uid) {
		// The existing user is already logged in.
		if ($user->uid == $uid) {
			//drupal_set_message(t('You are logged in as %user. <a href="!user_edit">Change your password.</a>', array('%user' => $user->name, '!user_edit' => url("user/$user->uid/edit"))));
			drupal_set_message(t('You are logged in as %user.', array('%user' => $user->name, '!user_edit' => url("user/$user->uid/edit"))));
		}
		// A different user is already logged in on the computer.
		else {
			$reset_link_account = user_load($uid);
			if (!empty($reset_link_account)) {
				drupal_set_message(t('Another user (%other_user) is already logged into the site on this computer, but you tried to use a one-time link for user %resetting_user. Please <a href="!logout">logout</a> and try using the link again.',
				array('%other_user' => $user->name, '%resetting_user' => $reset_link_account->name, '!logout' => url('user/logout'))));
			} else {
				// Invalid one-time link specifies an unknown user.
				drupal_set_message(t('The one-time login link you clicked is invalid.'));
			}
		}
		drupal_goto();
	}
	else {
		// Time out, in seconds, until login URL expires. 24 hours = 86400 seconds.
		$timeout = 86400;
		$current = REQUEST_TIME;
		// Some redundant checks for extra security ?

		//$users = user_load_multiple(array($uid), array('status' => '1'));
		$users = user_load_multiple(array($uid));
		if ($timestamp <= $current && $account = reset($users)) {
			// No time out for first time login.
			if ($account->login && $current - $timestamp > $timeout) {
				drupal_set_message(t('You have tried to use a one-time login link that has expired. Please request a new one using the form below.'));
				drupal_goto('user/password');
			}
			elseif ($account->uid && $timestamp >= $account->login && $timestamp <= $current && $hashed_pass == user_pass_rehash($account->pass, $timestamp, $account->login)) {
				$email_activation = false;
				if($account->status == 0){
					if($account->access == 0){ //patvirtinam e-mail'a
						$edit = array();
						$edit['status'] = 1;
						$account = user_save($account, $edit);
						drupal_set_title(t('Mano erdvė, el. pašto patvirtinimas'));
						$email_activation = TRUE;
						drupal_goto("account/activation/$uid/$timestamp/$hashed_pass/activate");
					}else{
						drupal_access_denied();
					}
				}

				// First stage is a confirmation form, then login
				if ($action == 'login') {
					watchdog('user', 'User %name used one-time login link at time %timestamp.', array('%name' => $account->name, '%timestamp' => $timestamp));
					// Set the new user.
					$user = $account;
					// user_login_finalize() also updates the login timestamp of the
					// user, which invalidates further use of the one-time login link.
					user_login_finalize();
					drupal_set_message(t('You have just used your one-time login link. It is no longer necessary to use this link to log in. Please change your password.'));
					// Let the user's password be changed without the current password check.
					$token = drupal_hash_base64(drupal_random_bytes(55));
					$_SESSION['pass_reset_' . $user->uid] = $token;
					drupal_goto('account/edit-password', array('query' => array('pass-reset-token' => $token)));
					//drupal_goto('account', array('query' => array('pass-reset-token' => $token)));
				}
				elseif ($action == 'activate') {
					watchdog('user', 'User %name used one-time login link at time %timestamp.', array('%name' => $account->name, '%timestamp' => $timestamp));
					// Set the new user.
					$user = $account;
					// user_login_finalize() also updates the login timestamp of the
					// user, which invalidates further use of the one-time login link.
					user_login_finalize();
					drupal_set_message(t('Sveikiname, Jūsų el. pašto adresas patvirtintas. Jūs automatiškai buvote prijungtas prie sistemos'));
					drupal_set_message(t('Jei Jūs jau turite @sitename i-lažybų sutartį (popierinę versiją), <a href="@url">priskirkite sutartį</a> ir iškarto galėsite naudotis lažybų svetainės paslaugomis.', array('@sitename' => variable_get('site_name'), '@url' => url('account/attach-agreement'))));
					drupal_goto('account');
				}
				else {
					if($email_activation){
						$form['message'] = array('#markup' => t('<p>Sveikiname, Jūsų el. pašto adresas patvirtintas.</p><p>Norėdami prisijungti prie paskyros, paspauskite žemiau esantį mygtuką "Prisijungti"</p>'));
					}else{
						$form['message'] = array('#markup' => t('<p>This is a one-time login for %user_name and will expire on %expiration_date.</p><p>Click on this button to log in to the site and change your password.</p>', array('%user_name' => $account->name, '%expiration_date' => format_date($timestamp + $timeout))));
						$form['help'] = array('#markup' => '<p>' . t('This login can be used only once.') . '</p>');
					}
					$form['actions'] = array('#type' => 'actions');
					$form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Log in'));
					$form['#action'] = url("user/reset/$uid/$timestamp/$hashed_pass/login");
					return $form;
				}
			}
			else {
				drupal_set_message(t('You have tried to use a one-time login link that has either been used or is no longer valid. Please request a new one using the form below.'));
				drupal_goto('user/password');
			}
		}
		else {
			// Deny access, no more clues.
			// Everything will be in the watchdog's URL for the administrator to check.
			drupal_access_denied();
		}
	}
}

/*
function account_register_done(){
	global $user;

	if(isset($_SESSION['#user'])){
		$account = $_SESSION['#user'];
	}else{
		$account = $user;
	}
	$html =
    '<strong>'.t('Sveikiname užsiregistravus').'</strong>'.'<br/>'.
	t('Mes Jums išsiuntėme laišką į Jūsų nurodytą pašto dėžutę :email.', array(':email' => (isset($account->mail) ? $account->mail : ''))).'&nbsp;'.
	t('Išsiųstame laiške rasite nuorodą, kurią paspaudę/nukopijavę Jūs patvirtinsite registracijoje nurodytą el. pašto adresą ir aktyvuosite paskyrą @sitename lažybų svetainėje.', array('@sitename' => variable_get('site_name'))).
    '<br/><br/>'.
	t('Jei per artimiausias kelias minutes negausite el. laiško, patikrinkite savo pašto dėžutės šiukšlės/brukalas (angl. junk mail, spam) katalogą arba pasinaudokite <a href="@url">pakartotinu instrukcijų siuntimu</a>.', array('@url' => url('user/activation'))).'<br/>'.
	t('Jei kyla problemų su registracija, prašome rašyti <a href="mailto:info@topsport.lt">info@topsport.lt</a> arba skambinkite +370 37 409105.');
	
	drupal_set_message($html);
	drupal_goto('node', array('query' => array('registration' => 'successful'), 'https' => FALSE));
	//return $html;
}*/

function account_account(){
	global $user; $account = clone $user;
	if ($account->uid) {
		if(account_need_agreement('access account agreement')){
			// return '';
			menu_set_active_item('account/preferences');
			//     }elseif(account_need_agreement('access account attach agreement')){
			//       menu_set_active_item('account/attach-agreement');
        } elseif(user_access('access account bets')){
				menu_set_active_item('account/bets/standart');
		} else{
			menu_set_active_item('account/info');
		}
		return menu_execute_active_handler(NULL, FALSE);
	}else{
		drupal_goto('user');
	}
}
/*
 function account_page($account){
 global $user; $account = clone $user;
 drupal_set_title(t('Mano erdvė'));
 if(account_need_agreement($account, 'access account agreement')){
 menu_set_active_item('account/agreement');
 }elseif(account_need_agreement($account, 'access account attach agreement')){
 menu_set_active_item('account/attach-agreement');
 }elseif(user_access('access account bets')){
 menu_set_active_item('account/bets/standart');
 }else{
 menu_set_active_item('account/info');
 }
 return menu_execute_active_handler(NULL, FALSE);
 $html = '';
 return theme('account_page', array('title' => t('Mano paskyra'), 'content' => $html));
 }
 */
function account_page_agreement(){
	global $user; $account = clone $user;
	drupal_set_title(t('Mano erdvė'));
	// keliaujam i nauja registracijos forma
	drupal_goto('accounts/registration-process');
	return ;
	/*
	$step = (isset($account->data['agreement_step']) ? $account->data['agreement_step'] : 0);
	$form_id = null;
	$form_extra = null;
	include_once drupal_get_path('module', 'user') . '/user.pages.inc';
	switch($step){
		case 0:
			$form_id = 'account_agreement_intro_form';
			$form = drupal_get_form($form_id, $account);
			return drupal_render($form);
			break;
		case 100:
			return account_page_attach_agreement();
			break;
		case 2:
			$form_id = 'account_agreement_form_confirm';
			$form = drupal_get_form($form_id, $account);
			break;
		case 3:
		case 3.2:
			//       $form_id = 'account_agreement_form_upload_document';
			//       $form = drupal_get_form($form_id, $account);
			//       break;
		case 3.1:
			//       $form = drupal_get_form('user_profile_form', $account, 'account', 'account_agreement_form_update');
			//       break;
		case 4:
			//       $form_id = 'account_agreement_form_waiting';
			//       $form = drupal_get_form($form_id, $account);
			drupal_goto('account/withdrawals');
			break;
		default:
			$html .= '<div class="go-back">« '.l(t('Grįžti atgal'), 'account/agreement/reset').'</div>';
			$form_id = 'account_agreement_form';
			$form = drupal_get_form($form_id, $account);
			$step = 1;
			break;
	}
	//   if($form_id == 'account_agreement_form'){
	//     $form['group_left'] = array(
	//     '#tree' => false,
	//     '#weight' => -50,
	//     );
	//     $form['group_left']['info_text'] = array(
	//       '#type' => 'markup',
	//       '#markup' => '<p>'.t('Norėdami naudotis lažybų svetaine ir atlikti statymus naudojantis "%sitename" lažybų svetaine, Jums reikia pasirašyti sutartį internetu ir galėsite naudotis visomis teikiamomis svetainės paslaugomis. Sutarties pasirašymo forma pateikta žemiau.', array('%sitename' => variable_get('site_name'))).'</p>',
	//       '#weight' => -20,
	//     );
	//     $form['group_left']['personal_info'] = $form['personal_info'];
	//     $form['group_left']['address_info'] = $form['address_info'];
	//     $form['group_left']['topsport_info'] = $form['topsport_info'];
	//     $form['group_left']['confirm_info'] = $form['confirm_info'];
	//     if(isset($form['document_info'])){
	//     $form['group_left']['document_info'] = $form['document_info'];
	//       unset($form['document_info']);
	//     }
	//     $form['group_left']['cancel_action'] = $form['cancel_action'];
	//     $form['group_left']['submit_action'] = $form['submit_action'];
	//     unset($form['personal_info'], $form['address_info'], $form['topsport_info'], $form['confirm_info'], $form['terms_of_use'], $form['account'], $form['cancel_action'], $form['submit_action'], $form['text_required']);
	//     $form['clearfix'] = array(
	//     '#type' => 'markup',
	//     '#markup' => '<div class="clear"></div>',
	//     '#weight' => 50,
	//     );
	//   }
	$form['timezone']['#access'] = FALSE;
	$form['locale']['#access'] = FALSE;
	$html .= render($form);
	return theme('account_page', array('title' => t('Sutarties pasirašymas, žingsnis @step iš 2', array('@step' => intval($step))), 'content' => $html));
	*/
}

/*
 * Senas sutarties priskyrimas, nebereikalinga.
 */
// function account_page_attach_agreement(){
// 	global $user; $account = clone $user;
// 	drupal_set_title(t('Mano erdvė'));
// 	$html = '';
// 	$html .= '<div class="go-back">« '.l(t('Grįžti atgal'), 'account/agreement/reset').'</div>';
// 	$form = drupal_get_form('account_attach_agreement_form', $account);
// 	$html .= render($form);
// 	return theme('account_page', array('title' => t('Esamos sutarties priskyrimas'), 'content' => $html));
// }

function account_page_document_upload(){
	global $user; $account = clone $user;
	drupal_set_title(t('Mano erdvė'));
	$html = '';
	$step = (isset($account->data['agreement_step']) ? $account->data['agreement_step'] : 3);
	$form_id = null;
	switch($step){
		case 3:
		case 3.2:
			$html .= '<div class="messages warning">'.t('Norint atlikti pinigų išmokėjimo operacijas, Jums reikia patvirtinti savo tapatybę, įkeliant skenuotą / fotografuotą geros kokybės, patvirtinantį dokumentą (pasas, tapatybės kortelė, vairuotojo pažymėjimas).').'</div>';
			$form_id = 'account_agreement_form_upload_document';
			break;
		case 3.1:
			include_once drupal_get_path('module', 'user') . '/user.pages.inc';
			$form = drupal_get_form('user_profile_form', $account, 'account', 'account_agreement_form_update');
			$form['timezone']['#access'] = FALSE;
			$form['locale']['#access'] = FALSE;
			return render($form);
			break;
		case 4:
			$form_id = 'account_agreement_form_waiting';
			break;
		default:
			$form_id = 'account_agreement_form_upload_document';
			$step = 3;
			break;
	}
	$form = drupal_get_form($form_id, $account);
	$html .= render($form);
	return $html;/*theme('account_page', array('title' => t('Dokumento pasirašymas'), 'content' => $html));*/
}

function account_page_agreement_upload($pdf = false){
	global $user; $account = clone $user;
	drupal_set_title(t('Mano erdvė'));
	if($pdf){
		account_page_get_agreement();
	}else{
		$html = '';
		$html .= render(drupal_get_form('account_agreement_form_upload_agreement', $account));
		return theme('account_page', array('title' => t('Sutarties įkėlimas'), 'content' => $html));
	}
}

function account_page_get_agreement(){
	global $user; $account = clone $user;
	require_once(DRUPAL_ROOT . '/' . drupal_get_path('module', 'account') . '/account.pdf.class.inc');

	$pdf = new AccountPDF();
	$pdf->SetMargins(0, 0, 0);
	$pdf->SetAutoPageBreak(false, 0);
	$pdf->setFontSubsetting(false);

	$pdf->setSourceFile(DRUPAL_ROOT . '/sites/all/libraries/lazybu-sutartis.pdf');

	$pdf->AddPage();
	$pdf->useTemplate($pdf->importPage(1));
	$pdf->SetFont("freeserif", "", 5);
	$i = 0;/*
	while($i <= 200){
	//$pdf->SetXY($i, 0); $pdf->Write(5, $i, '', false, '', TRUE);
	$j = 0;
	while($j < 300){
	$pdf->SetXY($i, $j);
	$pdf->Write(5, $i.':'.$j, '', false, '', TRUE);
	$j += 5;
	}
	$i += 5;
	}*/
	$pdf->SetFont("freeserif", "", 11);


	// Cell($w, $h=0, $txt='', $border=0, $ln=0, $align='', $fill=false, $link='', $stretch=0, $ignore_min_height=false, $calign='T', $valign='M') {
	$pdf->SetFillColor(255, 255, 255);
	$pdf->SetXY(114, 22);
	$pdf->Cell(50, 6, '', 0, 0, 'C', true);

	$xcoord = 114;
	$ContractIdentifier = account_generateContractIdentifier($account);

	$k = 0;
	while(isset($ContractIdentifier[$k])){
		$pdf->SetXY($xcoord, 22.5);
		$pdf->Cell(4, 5, $ContractIdentifier[$k], 1, 0, 'C', true);
		$xcoord += 4.5;
		$k++;
	}
	/*
	 $pdf->SetXY(118, 22.5);
	 $pdf->Cell(4, 5, '2', 1, 0, 'C', true);
	 $pdf->SetXY(122, 22.5);
	 $pdf->Cell(4, 5, '3', 1, 0, 'C', true);
	 $pdf->SetXY(126, 22.5);
	 $pdf->Cell(4, 5, '4', 1, 0, 'C', true);
	 $pdf->SetXY(130, 22.5);
	 $pdf->Cell(4, 5, '-', 1, 0, 'C', true);
	 */

	$pdf->SetFont("freeserif", "", 9);
	$pdf->SetXY(81, 28);
	$pdf->Write(0, substr($account->data['ContractDate'], 3, 1));

	$pdf->SetXY(93, 28);
	$pdf->Write(0, account_pdf_month(format_date(strtotime($account->data['ContractDate']), 'custom', 'm')));

	$pdf->SetXY(135, 28);
	$pdf->Write(0, format_date(strtotime($account->data['ContractDate']), 'custom', 'd'));


	$pdf->SetXY(52, 49.5);
	$pdf->Write(0, $account->data['firstname'].' '.$account->data['surname'].' ('.$account->data['personalId'].')');

	$pdf->SetXY(175, 49.5);
	$pdf->Write(0, $account->data['birthday']);

	$pdf->SetXY(32, 53.5);
	$pdf->Write(0, $account->data['street'].' '.$account->data['house'].($account->data['flat']? '-'.$account->data['flat']:''));

	$pdf->SetXY(140, 53.5);
	$pdf->Write(0, $account->mail);

	$bet_center = betting_center_centers(null, null, $account->data['betcenter']);

	$bet_center_city = betting_center_cities($bet_center->bccid, true);
	$pdf->SetXY(86, 57);
	$pdf->Write(0, $bet_center->address.', '.$bet_center_city->title);

	$pdf->Output('lazybu-sutartis.pdf', 'D');
	exit;
}

function account_betDetailtTypes($key){

	$types = array(
    '0' => t('Paprastas'),
    '1' => t('Kombinuotas'),
    '2' => t('Sisteminis'),
	);
	return $types[$key];

}

function account_page_bets($CardNumber = null){
	global $user; $account = clone $user;
	drupal_set_title($title = t('Mano erdvė'));

	$html = '';
	//$is_mantanence = isset($user->data['tsuid']) && $user->data['tsuid'] == 726 ? false : true;
    $is_mantanence = false;
	if(!$is_mantanence){
		if($CardNumber){
			$html .= account_page_bets_detail($CardNumber);
			$title = t('Statymo informacija');
		}else{
			$html .= account_page_bets_list();
			$title = t('Mano statymai');
		}
        drupal_set_title($title);
	} else {
		$html = 'Vyksta techninė profilaktika, ši puslapio dalis bus prieinama po keletos minučių.';
	}
	return theme('account_page', array('title' => $title, 'content' => $html));
}

function account_page_bets_detail($CardId = null){
	global $user; $account = clone $user;
	global $language;
	$card = array();
	if($tsuid = userinfo_get_topsportId($account)){
		if ($cached = cache_get('account_page_bets:'.$tsuid.':card:'.$language->language.':'.$CardId, 'cache')){
			$card = $cached->data;
		}else{
		$card = topsport_getCardDetails($tsuid, $CardId, $language->language);
			if(isset($card['Events']) && sizeof($card['Events'])){
			cache_set('account_page_bets:'.$tsuid.':card:'.$language->language.':'.$CardId, $card, 'cache', strtotime('+1min'));
            
		}
		}
	}    
	if(empty($card) || isset($card['Message'])){ 
        drupal_set_message(t('Lažybų kortelė nerasta'), 'warning');
        drupal_goto('account/bets/standart');         
    }
	$html = '';
	if(!isset($card['CardStatus'])){
		$card['CardStatus']['CardStatusId'] = '0';
	}
	$betStatus = account_cardStatusesClass($card['CardStatus']['CardStatusId']);
	if(!isset($card['Events']['Event']['0'])){
		$card['Events']['Event'] = array($card['Events']['Event']);
	}
	$event_cnt = count($card['Events']['Event']);
	$type = ($event_cnt == 1 ? 0 : $card['BetTypeId']);
	$typeTitle = account_betDetailtTypes($type);
	$rows = array(
	array(
	array(
      'header' => TRUE,
      'data' => t('Kortelės Nr.'),
	), array(
        'data' => (isset($card['CardNumber']) ? $card['CardNumber'] : t('Kortelė dar neatspausdinta')),
	)
	),
	array(
	array(
      'header' => TRUE,
      'data' => t('Statymo tipas'),
	), array(
        'data' => $typeTitle,
	)
	),
	array(
	array(
      'header' => TRUE,
      'data' => t('Statymo data'),
	), array(
        'data' => date('Y-m-d H:i:s', strtotime($card['BetTimeStamp'])),
	)
	),
	array(
	array(
      'header' => TRUE,
      'data' => t('Būsena'),
	), array(
        'data' => isset($card['CardStatus']['CardStatusDescription']) ? $card['CardStatus']['CardStatusDescription'] : t('Priimta'),
	)
	),
	array(
	array(
      'header' => TRUE,
      'data' => t('Statymo suma'),
	), array(
        'data' => account_currency_convert_string($account->uid, $card['BetAmount']),
	)
	),
	array(
	array(
      'header' => TRUE,
      'data' => t('Koeficientas'),
	), array(
        'data' => number_format($card['BetCoefficient'], 2),
	)
	),
	array(
	array(
      'header' => TRUE,
      'data' => t('Laimėjimas'),
	), array(
        'data' => ($betStatus == 'won' ? account_currency_convert_string($account->uid, $card['WinAmount']) : ($betStatus == 'waiting' ? theme('image', array('title' => t('Laukiama rezultato'), 'path' => 'sites/all/img/icons/wait.png')) : '—')),
        'class' => array('winamount')
	)
	),
	);
	$html .= theme('table', array('rows' => $rows, 'attributes' => array('style' => array('width: 300px;'), 'class' => array($betStatus))));


	$eventsRows = array();
	$eventsStatus = array();
	$i = -1;
	$letters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz';
	foreach($card['Events']['Event'] as $event){
		$event_res = array();
		if(isset($event['Result']) AND !empty($event['Result'])){
			if(isset($event['Result']['CorrectSelections'])){
				if(isset($event['Result']['CorrectSelections']['CorrectSelection'])){
					if(is_array($event['Result']['CorrectSelections']['CorrectSelection'])){
						foreach($event['Result']['CorrectSelections']['CorrectSelection'] as $res){ $event_res[] = (string)$res; }
					}else{
						$event_res[] = $event['Result']['CorrectSelections']['CorrectSelection'];
					}
				}
			}elseif(isset($event['Result']['ResultValue1'])){
				$event_res[] = $event['Result']['ResultValue1'].(isset($event['Result']['ResultValue2']) ? ':'.$event['Result']['ResultValue2'] : '');
			}
		}
		$betStatus = account_cardStatusesClass($event['EventStatus']['StatusId']);
		$eventsStatus[substr($letters, ++$i, 1)] = $betStatus;
		$eventRow = array(
      'class' => array($betStatus),
      'data' => array(
		array(
          'data' => ($type == 2 ? substr($letters, $i, 1) : ''),
		),
		array(
          'data' => date('Y-m-d H:i', strtotime($event['EventDate'].' '.$event['EventTime'])),
		),
		array(
          'class' => array('icon'),
          'data' => offers_getAreaIcon((int)$event['Area']['AreaId'], FALSE, '22px', $event['Area']['AreaName']),
		),
		array(
          'data' => (string)$event['EventName'],
          'class' => array('text-center'),
          'title'=> (string)$event['Area']['AreaName'].' » '.(string)$event['Module']['ModuleName']. ' » '.(string)$event['GameType']['GameTypeName'].t(', įvykio nr. @number', array('@number' => (string)$event['EventName'])),
		),
		array(
          'data' => $event['EventString'],
          'title'=> (string)$event['Area']['AreaName'].' » '.(string)$event['Module']['ModuleName']. ' » '.(string)$event['GameType']['GameTypeName'].t(', įvykio nr. @number', array('@number' => (string)$event['EventName'])),
		),
		array(
          'data' => $event['GameType']['GameTypeName'],
		),
		array(
          'data' => $event['Selection'],
		),
		array(
          'data' => (!empty($event_res) ? implode(', ', $event_res) : theme('image', array('title' => t('Laukiama rezultato'), 'path' => 'sites/all/img/icons/wait.png'))),
		),
		array(
          'data' => $event['Coefficient'],
          'class' => array('text-right'),
		),
		),
		);
		$eventsRows[] = $eventRow;
	}

	$variables = array(
    'header' => array(
	array(
        'data' => '',
	),
	array(
        'data' => t('Įvykio pabaigos data'),
	),
	array(
        'class' => array('icon'),
        'data' => '',
	),
	array(
        'data' => t('Įvykio nr.'),
        'class' => 'text-center',
	),
	array(
        'data' => t('Įvykis'),
	),
	array(
        'data' => t('Statymo tipas'),
	),
	array(
        'data' => t('Statymas'),
	),
	array(
        'data' => t('Rezultatas'),
	),
	array(
        'data' => t('Koeficientas'),
        'class' => array('text-right'),
	),
	),
    'rows' => $eventsRows,
    'attributes' => array('class'=> array('lost-won-colors')),
    'caption' => NULL,
    'colgroups' => array(),
    'sticky' => TRUE,
    'empty' => t('Įvykių nėra'),
	);
	$html .= '<h3>'.t('Pasirinkti įvykiai').'</h3>';
	$html .= theme('table', $variables);

	if($type == 2){
		$combinationRows = array();

		foreach($card['Combinations']['Combination'] as $Combination){
			$CombinationStatus = '';
			$CombinationNameLetters = preg_split("//", $Combination['CombinationName'], -1, PREG_SPLIT_NO_EMPTY);
			foreach($CombinationNameLetters as $eventLetter){
				if($eventLetter != ':'){
					$CombinationStatus = $eventsStatus[$eventLetter];
					if($CombinationStatus == 'lost'){ break; }
				}
			}
			$combinationRow = array(
        'class' => array($CombinationStatus),
        'data' => array(
			array(
            'data' => $Combination['CombinationName'],
			),
			array(
            'data' => $Combination['BetCoefficient'],
            'class' => array('text-right'),
			),
			array(
            'data' => account_currency_convert_string($account->uid, $Combination['BetAmount']),
            'class' => array('text-right'),
			),
			),
			);

			$combinationRows[] = $combinationRow;
		}
		$variables = array(
      'header' => array(
		array(
          'data' => t('Kombinacija'),
		),
		array(
          'data' => t('Koeficientas'),
          'class' => array('text-right'),
		),
		array(
          'data' => t('Statymo suma'),
          'class' => array('text-right'),
		),
		),
      'rows' => $combinationRows,
      'attributes' => array('style' => array('width: auto;'), 'class'=> array('lost-won-colors')),
      'caption' => NULL,
      'colgroups' => array(),
      'sticky' => TRUE,
      'empty' => t('Kombinacijų nėra'),
		);
		$html .= '<h3>'.t('Sisteminio statymo kombinacijos').'</h3>';
		$html .= theme('table', $variables);
	}


	//$html .= '<pre>'.print_r($card, true).'</pre>';
	return $html;
}

function account_page_bets_list($page = 1, $onlytr = false) {
    global $user, $language;
    $account = clone $user;
    $has_more = &drupal_static('account_bet_has_more', FALSE);
    $html = '';
    $rows = array();
    $pagesize = 50;
    if ($tsuid = userinfo_get_topsportId($account)) {
        $filter = account_bets_filter_result();
        switch ($filter['date_type']) {
            case 'today':
                $dateFrom = strtotime('today');
                $dateTo = strtotime('tomorrow');
                break;
            case 'yesterday':
                $dateFrom = strtotime('yesterday');
                $dateTo = strtotime('today');
                break;
            case '7days':
                $dateFrom = strtotime('-7 days');
                $dateTo = strtotime('tomorrow');
                break;
            case 'period':
                $dateFrom = strtotime($filter['date_from']);
                $dateTo = strtotime('+1 day', strtotime($filter['date_to']));
                drupal_add_css('div.container-inline-date{display:inline !important;}', 'inline');
                break;
            case '30days':
            default:
                $dateFrom = strtotime('- 30 days');
                $dateTo = strtotime('tomorrow');
                break;
        }
        $_key = array('account_page_bets',$tsuid,'bets',$filter['bet_state'],date('Y-m-d', $dateFrom),date('Y-m-d', $dateTo));
        $_key = implode(':', $_key);
        $cached = cache_get($_key, 'cache');
        if ($cached) {
            $bets = $cached->data;            
        } else {
            $bets = topsport_GetUserBets($tsuid, 1, 500, $filter['bet_state'], $dateFrom, $dateTo);
            
            if (isset($bets['Card']) && count($bets['Card'])) {
                cache_set($_key, $bets, 'cache', strtotime('+10 min'));                
            }
        }
        if (!$onlytr) {
                $account_bets_filter = drupal_get_form('account_bets_filter');
                $html .= drupal_render($account_bets_filter);
        }
        if (isset($bets['Card']) && sizeof($bets['Card'])) {
            


            $k = 1;
            $flip = array(
              'even' => 'odd',
              'odd' => 'even',
            );
            $class = 'even';
            if (isset($bets['Card']['CardId'])) {
                $bets['Card'] = array('0' => $bets['Card']);
            }
            foreach ($bets['Card'] as $item) {
                if (!isset($item['CardStatus'])) {
                    $item['CardStatus']['CardStatusId'] = '0';
                }
                $i = 0;
                if (isset($item['Events']['Event']['EventId'])) {
                    $item['Events']['Event'] = array($item['Events']['Event']);
                }
                $event_cnt = sizeof($item['Events']['Event']);
                $type = ($event_cnt == 1 ? 0 : $item['BetTypeId']);
                $typeTitle = account_betDetailtTypes($type);
                if (!empty($filter) && $filter['bet_type'] != 'all') {
                    if ($type != $filter['bet_type']) {
                        continue;
                    }
                }
                if ($filter['bet_state']) {
                    if (!in_array($item['CardStatus']['CardStatusId'], account_cardStatuses($filter['bet_state']))) {
                        continue;
                    }
                }
                if (strtotime($item['BetTimeStamp']) < $dateFrom OR strtotime($item['BetTimeStamp']) > $dateTo) {
                    continue;
                }

                /*if ($k <= (($page - 1) * $pagesize)) {
                    $k++;
                    continue;
                }
                if ($k > ($page * $pagesize)) {
                    $has_more = true;
                    break;
                }*/

                $events = $item['Events']['Event'];

                foreach ($events as $event) {
                    $row = array();
                    if ($i === 0) {
                        $class = $flip[$class];
                        $bet_card_link = '<br>' . l('#' . (isset($item['CardNumber']) ? $item['CardNumber'] : t('Kortelė dar neatspausdinta')), 'account/bets/standart/' . $item['CardId']);
                        $bet_card_type = '<span class="bet-type">'.(isset($item['CardId']) ? l($typeTitle, 'account/bets/standart/' . $item['CardId'], array('html' => true, 'attributes' => array('class' => array('no-underline')))) : $typeTitle).'</span>' ;
                        $row['class'] = array('bet');
                        $row['data'][] = array('data' => $bet_card_type.date('Y-m-d H:i', strtotime($item['BetTimeStamp'])) . ' ' . $bet_card_link, 'rowspan' => $event_cnt);
                    } else {
                        $row['class'] = array('bet-detail');
                    }
                    $row['class'][] = $class;
                    $betStatus = account_cardStatusesClass($item['CardStatus']['CardStatusId']);
                    $row['class'][] = $betStatus;                    
                    $row['data'][] = array('data' => '<span class="tssporticon tssporticon-s' . (int) $event['Area']['AreaId'] . '"></span>' . $event['EventString'], 'title' => (string) $event['Area']['AreaName'] . ' » ' . (string) $event['Module']['ModuleName'] . ' » ' . (string) $event['GameType']['GameTypeName'] . t(', įvykio nr. @number', array('@number' => (string) $event['EventName'])));
                    $row['data'][] = $event['Selection'];
                    $event_res = array();
                    if (isset($event['Result']) AND !empty($event['Result'])) {
                        if (isset($event['Result']['CorrectSelections'])) {
                            if (isset($event['Result']['CorrectSelections']['CorrectSelection'])) {
                                if (is_array($event['Result']['CorrectSelections']['CorrectSelection'])) {
                                    foreach ($event['Result']['CorrectSelections']['CorrectSelection'] as $res) {
                                        $event_res[] = (string) $res;
                                    }
                                } else {
                                    $event_res[] = $event['Result']['CorrectSelections']['CorrectSelection'];
                                }
                            }
                        }
                    }
                    $row['data'][] = (!empty($event_res) ? implode(', ', $event_res) : theme('image', array('title' => t('Laukiama rezultato'), 'path' => 'sites/all/img/icons/wait.png')));
                    if ($i === 0) {
                        $row['data'][] = array('data' => account_currency_convert_string($account->uid, $item['BetAmount']), 'rowspan' => $event_cnt, 'class' => array('text-right'));
                        $row['data'][] = array('data' => number_format($item['BetCoefficient'], 2, '.', ' '), 'rowspan' => $event_cnt, 'class' => array('text-right'));
                        $row['data'][] = array('data' => ($betStatus == 'won' ? account_currency_convert_string($account->uid, $item['WinAmount']) : ($betStatus == 'waiting' ? theme('image', array('title' => t('Laukiama rezultato'), 'path' => 'sites/all/img/icons/wait.png')) : '0' . account_currency_convert_string(false, 0, 'LTL', 0))), 'rowspan' => $event_cnt, 'class' => array('text-right', 'winamount'));
                    }
                    $i++;
                    $rows[] = $row;
                }
                $k++;
            }
        }
    }
    $variables = array(
      'header' => array(
        array(
          'data' => t('Data'),
        ),
        /* array(
          'data' => t('Kortelės Nr.'),
          ), */
        /*array(
          'data' => t('Statymo tipas'),
        ),*/
        /* array(
          'class' => array('icon'),
          'data' => '',
          ), */
        array(
          'data' => t('Įvykis'),
        ),
        array(
          'data' => t('Statymas'),
        ),
        array(
          'data' => t('Rezultatas'),
        ),
        array(
          'data' => t('Statymo suma'),
          'class' => array('text-right'),
        ),
        array(
          'data' => t('Koeficientas'),
          'class' => array('text-right'),
        ),
        array(
          'data' => t('Laimėjimas'),
          'class' => array('text-right'),
        ),
      ),
      'rows' => $rows,
      'attributes' => array('id' => array('my-bets-list'), 'class' => array('account-bets', 'lost-won-colors', 'table-hover')),
      'caption' => NULL,
      'colgroups' => array(),
      'sticky' => TRUE,
      'empty' => t('Statymų nėra'),
    );
    if ($onlytr) {
        $html .= account_format_rows($variables);
    } else {
        drupal_add_library('system', 'drupal.ajax');
        drupal_add_js('jQuery(document).ready(function () { Drupal.ajax.prototype.commands.accountAppend =  function (ajax, response, status) {jQuery(response.selector).append(response.value);}; });', 'inline');
        $html .= theme('table', $variables);
        if ($has_more) {
            $html .= '<div id="div-get-more-bets" class="text-center">' . l(t('Daugiau statymų'), 'account/bets/standart/more/' . ($page + 1) . '/nojs/', array('attributes' => array('id' => array('get-more-bets'), 'class' => array('use-ajax', 'btn btn-primary')))) . '</div>';
        }
    }

    return $html;
}

function account_page_bets_more($page = 1, $ajax = 'nojs'){
	global $user; $account = clone $user;
	$list = account_page_bets_list($page, ($ajax == 'ajax'));
	if($ajax == 'ajax'){
		$commands = array();
		$commands[] = ajax_command_accountAppend('#my-bets-list tbody', $list);
		$has_more = &drupal_static('account_bet_has_more', FALSE);
		if($has_more){
			$commands[] = ajax_command_replace('#div-get-more-bets', '<div id="div-get-more-bets">'.l(t('Daugiau statymų'), 'account/bets/standart/more/'.($page+1).'/nojs/', array('attributes' => array('id' => array('get-more-bets'), 'class' => array('use-ajax')))).'</div>');
		}else{
			$commands[] = ajax_command_remove('#div-get-more-bets');
		}
		print ajax_render($commands);
		exit;
	}else{
		return $list;
	}
}

function ajax_command_accountAppend($selector, $value){
	return array(
    'command' => 'accountAppend',
    'selector' => $selector,
    'value' => $value,
	);
}

function account_format_rows($variables){
	$rows = $variables['rows'];
	$attributes = $variables['attributes'];

	$output = '';

	$header = null;
	$ts = array();
	// Format the table rows:
	if (count($rows)) {
		$flip = array(
      'even' => 'odd',
      'odd' => 'even',
		);
		$class = 'even';
		foreach ($rows as $number => $row) {
			$attributes = array();

			// Check if we're dealing with a simple or complex row
			if (isset($row['data'])) {
				foreach ($row as $key => $value) {
					if ($key == 'data') {
						$cells = $value;
					}
					else {
						$attributes[$key] = $value;
					}
				}
			}
			else {
				$cells = $row;
			}
			if (count($cells)) {
				// Add odd/even class
				if (empty($row['no_striping'])) {
					$class = $flip[$class];
					$attributes['class'][] = $class;
				}

				// Build row
				$output .= ' <tr' . drupal_attributes($attributes) . '>';
				$i = 0;
				foreach ($cells as $cell) {
					$cell = tablesort_cell($cell, $header, $ts, $i++);
					$output .= _theme_table_cell($cell);
				}
				$output .= " </tr>\n";
			}
		}
	}
	return $output;
}

function account_page_deposits(){
	global $user; $account = clone $user;
	drupal_set_title(t('Mano erdvė'));
	$title = t('Sąskaitos papildymas');    
	if(topsport_searchDepositOperationsAreAllowed()){
		$form = drupal_get_form('account_deposits_form', $account);
		$title = t('Sąskaitos papildymas, žingsnis @step iš 3', array('@step' => $form['#step']));
		$html = drupal_render($form);
		if(isset($_GET['promo']) && $_GET['promo'] == 'discount'){
			$Wallets = account_user_data($user->uid);			
			drupal_set_message(t('
					<h1>Papildyk savo balansą ir gauk @DiscountPercent% nuolaidą statymams atlikti</h1>
					<p>Pasiūlymas galioja iki: <strong>@ValidUntil</strong><br>
                            Minimalus bendras kortelės statymo koeficientas: <strong>@MinOdds</strong><br>
                            Nuolaidos nustos galioti po <strong>@UsedCountLeft</strong> statymų arba išnaudojus <strong>@LeftAmount LTL</strong> nuolaidų vertę, arba pasibaigus nurodytam terminui.</p>
					<p class="small">Nuolaidos galioja tik sporto lažyboms iki varžybų pradžios. Nuolaidų sąlygas bendrovė pasilieka teisę keisti be įspėjimo.</p>',array('@DiscountPercent' => $Wallets->discount_max_cover_percent, '@MinOdds' => $Wallets->discount_min_total_coef, '@ValidUntil' => $Wallets->discount_valid_to, '@UsedCountLeft' => $Wallets->discount_max_used_count - $Wallets->discount_used_count, '@LeftAmount' => $Wallets->discount_current_amount)), 'warning');
		}
	}else{
		$html = '
    <div class="messages warning">
        '.t('Pinigų įnešimo operacijos laikinai sustabdytos, prašome bandyti vėliau.').'
    </div>';
	}
    drupal_set_title($title);

	return theme('account_page', array('title' => $title, 'content' => $html));
}

function account_deposits_actions($action){
	drupal_set_title(t('Mano erdvė'));	
	global $user;	
	
	$meth_name = isset($_SESSION['pay_method']) ? $_SESSION['pay_method'] : '-';
	$meth_amount = isset($_SESSION['pay_amount']) ? $_SESSION['pay_amount'] : '-';
	switch ($action){
		case 'accept':
			$_SESSION['ga_push'][] = array('_trackEvent',
    'Sąskaitos papildymas',
    'Atliktas',$meth_name,	$meth_amount);
			drupal_set_message(t('Jūsų mokėjimas buvo priimtas.'));
			break;
		case 'cancel':
			$_SESSION['ga_push'][] = array('_trackEvent',
    'Sąskaitos papildymas',
    'Atšauktas',$meth_name,	$meth_amount);
			drupal_set_message(t('Jūsų mokėjimas buvo atšauktas.'), 'error');
			break;
	}
	if($user->uid > 0){
		drupal_goto('account/deposits');
	}else{
		drupal_goto('account');
	}
}

function account_page_preferences(){
	global $user; $account = clone $user;
	drupal_set_title(t('Mano erdvė'));
	$html = '';

	$form = drupal_get_form('account_preferences_form', $account);
	$form['locale']['language']['#title'] = t('Atvaizdavimo kalba');
	$form['locale']['language']['#description'] = t('Kalba pagal nutylėjimą skirta atvaizduoti svetainės, elektroninių laiškų turinius');
	$form['timezone']['timezone']['#description'] = t('Pagal pasirinktą laiko juostą, bus atvaizduojama data ir laikas visoje svetainėje');
	$html = render($form);
    drupal_set_title(t('Nustatymai'));
	return theme('account_page', array('title' => t('Nustatymai'), 'content' => $html));
}

function account_page_withdrawals($method = false, $first = false, $action = false, $baid = false){
	global $user; $account = clone $user;
	drupal_set_title(t('Mano erdvė'));
	if(variable_get('account_roles_topsport_full', null) AND isset($account->roles[variable_get('account_roles_topsport_full', null)])){
		if(!topsport_searchDepositOperationsAreAllowed()){
			$html = '<div class="messages warning">'.t('Pinigų išmokėjimo operacijos šiuo metu nėra galimos. Pamėginkite vėliau').'</div>';
			return theme('account_page', array('title' => t('Pinigų išmokėjimas'), 'content' => $html));
		}
		if(!$method){
			return account_page_withdrawals_page();
		}
		switch($method){
			case 'bank-transfer':
				return account_page_withdrawals_page_banktransfer($first, $action, $baid);
				break;
			case 'moneybookers':
				return account_page_withdrawals_page_moneybookers();
				break;
			default:
				drupal_goto('account/withdrawals');
				break;
		}
	}else{
		$html = account_page_document_upload($account);
		$title = t('Pinigų išmokėjimas');
		return theme('account_page', array('title' => $title, 'content' => $html));
	}
}

function account_page_withdrawals_page(){

	$html = account_withdrawals_methods_table();

	$title = t('Pinigų išmokėjimas');
    drupal_set_title($title);
	return theme('account_page', array('title' => $title, 'content' => $html));
}


function account_page_withdrawals_page_banktransfer($first = false, $action = false, $baid = false){
	global $user; $account = clone $user;
	$html = '';

	if($first){
		$html = '';
		$updatebankaccounts = null;
		if($action){
			if($baid){
				if($action == 'remove'){
					account_removeBankAccount($account->uid, $baid);
					drupal_goto('account/withdrawals/bank-transfer/bank-accounts');
					exit;
				}elseif($action == 'update'){
					$updatebankaccounts = account_getbankaccounts($account->uid,  $baid);
					if(!$updatebankaccounts){ drupal_goto('account/withdrawals/bank-transfer/bank-accounts'); }
					$html .= render(drupal_get_form('account_bank_accounts', $account, $updatebankaccounts));
					return theme('account_page', array('title' => t('Koreguoti banko sąskaitą'), 'content' => $html));
				}
			}else{
				drupal_goto('account/withdrawals/bank-transfer/bank-accounts');
				exit;
			}
		}else{
			$bankaccounts = account_getbankaccounts($account->uid);
			$rows = array();
			foreach($bankaccounts as $bankaccount){
				$rows[] = array(
				array('data' => $bankaccount->bankname),
				array('data' => $bankaccount->iban),
				array('data' => $bankaccount->swift),
				array('data' => l(t('Remove'), 'account/withdrawals/bank-transfer/bank-accounts/remove/'.$bankaccount->baid)),
				array('data' => l(t('Update'), 'account/withdrawals/bank-transfer/bank-accounts/update/'.$bankaccount->baid)),
				);
			}
			$header = array(
			array('data' => t('Banko pavadinimas')),
			array('data' => t('Banko sąskaitos nr.')),
			array('data' => t('SWIFT')),
			array('data' => ''),
			array('data' => ''),
			);
			$html .= theme('table', array('rows' => $rows, 'header' => $header, 'empty' => t('Nėra registruotų banko sąskaitų')));
			$html .= '<h3>'.t('REGISTRUOTI NAUJĄ SĄSKAITĄ').'</h3>';
			$html .= render(drupal_get_form('account_bank_accounts', $account));
		}
		return theme('account_page', array('title' => t('Banko sąskaitos'), 'content' => $html));
	}else{
		$form = drupal_get_form('account_withdrawals_form', $account);
		$html .= render($form);
		$title = t('Pinigų išmokėjimas, žingsnis @step iš 2', array('@step' => $form['#step']));
        drupal_set_title($title);
		return theme('account_page', array('title' => $title, 'content' => $html));
	}
}

function account_page_withdrawals_page_moneybookers(){
	global $user; $account = clone $user;
	$html = '';
	$html .= '';
	$form = drupal_get_form('account_withdrawals_moneybookers_form', $account);
	$html .= render($form);
	$title = t('Pinigų išmokėjimas, žingsnis @step iš 2', array('@step' => $form['#step']));
    drupal_set_title($title);
	return theme('account_page', array('title' => $title, 'content' => $html));

}

function account_page_history($action = false, $depositId = false){
	global $user; $account = clone $user;
	global $language;
	list($lang_code) = explode('-', $language->language);
	drupal_set_title(t('Mano erdvė'));
	$session = isset($_SESSION['account_history_filter']) ? $_SESSION['account_history_filter'] : array('');
	$html = '';
	$rows = array();
	if(!isset($session['date_type'])){ $session['date_type'] = '30days'; }
	switch($session['date_type']){
		case 'today':
			$dateFrom = strtotime('today');
			$dateTo = strtotime('tomorrow');
			break;
		case 'yesterday':
			$dateFrom = strtotime('yesterday');
			$dateTo = strtotime('today');
			break;
		case '7days':
			$dateFrom = strtotime('-7 days');
			$dateTo = strtotime('tomorrow');
			break;
		case 'period':
			$dateFrom = strtotime($session['date_from']);
			$dateTo = strtotime($session['date_to']);
			break;
		case '30days':
		default:
			$dateFrom = strtotime('- 30 days');
			$dateTo = strtotime('tomorrow');
			break;
	}
	if($tsuid = userinfo_get_topsportId($account)){
		if($action && $action == 'cancel' && $depositId){
			$result = topsport_cancelDepositOutTopSport(array('user_id' => $tsuid, 'deposit_out_id' => $depositId));
			if($result['Code'] == 0){
				drupal_set_message(t('Išmokėjimo operacija atšaukta'));
			}else{
				$error = (isset($result['Description'][$language->language]) ? $result['Description'][$language->language] : $result['Description']['Default']);
				drupal_set_message($error, 'error');
			}
			userinfo_updateUser($account, true, true);
			drupal_goto('account/history');
		}
		if ($cached = cache_get('account_page_history:'.$tsuid, 'cache')) {
			$history = $cached->data;
		}else{
			$history = topsport_searchDepositsReport($tsuid);
			if(isset($history['ClientUser'])){
				cache_set('account_page_history:'.$tsuid, $history, 'cache', strtotime('+5 min'));
			}
		}
		$account_history_filter = drupal_get_form('account_history_filter');
		$html .= drupal_render($account_history_filter);
		if(isset($history['ClientUser'])){
			if($history['ClientUser']['AllDepositOperations'] AND sizeof($history['ClientUser']['AllDepositOperations'])){
				$paymentTypes = array(
		          'income' => t('Įmoka'),
		          'outcome' => t('Išmoka'),
				);
				$symbols = array(
		          'income' => '+',
		          'outcome' => '-',
				);
				if(isset($history['ClientUser']['AllDepositOperations']['DepositOperation']['DepositId'])){
					$history['ClientUser']['AllDepositOperations']['DepositOperation'] = array($history['ClientUser']['AllDepositOperations']['DepositOperation']);
				}
				foreach($history['ClientUser']['AllDepositOperations']['DepositOperation'] as $item){
					if(isset($session['payment_type']) and $session['payment_type'] != 'all'){
						if($item['PaymentType'] != $session['payment_type']) continue;
					}
					if(strtotime($item['OrderTimestamp']) < $dateFrom OR strtotime($item['OrderTimestamp']) > $dateTo){
						continue;
					}
					$rows[] = array(
					date('Y-m-d H:i:s', strtotime($item['OrderTimestamp'])),
					$paymentTypes[$item['PaymentType']],
					array('data' => (($item['DepositAmount'] < 0 AND $item['PaymentType'] == 'income') ? '' : $symbols[$item['PaymentType']]).account_currency_convert_string($account->uid, $item['DepositAmount'], $item['CurrencyName']), 'class' => array((($item['DepositAmount'] < 0 AND $item['PaymentType'] == 'income') ? 'outcome' : $item['PaymentType']), 'text-right')),
					array('data' => account_currency_convert_string($account->uid, $item['BalanceAmountBeforeOperation'], $item['CurrencyName']), 'class' => array('text-right')),
					(isset($item['State']['StateDescription'][drupal_strtoupper($lang_code)]) ? $item['State']['StateDescription'][drupal_strtoupper($lang_code)] : $item['State']['StateDescription']['Default']),
					($item['State']['StateId'] == 0 && $item['PaymentType'] == 'outcome' ? l(t('Atšaukti išmokėjimo operaciją'), 'account/history/cancel/'.$item['DepositId']).'<br/>' : '').
					(isset($item['Error']['ErrorDescription'][drupal_strtoupper($lang_code)]) ? $item['Error']['ErrorDescription'][drupal_strtoupper($lang_code)] : (isset($item['Error']['ErrorDescription']['Default']) ? $item['Error']['ErrorDescription']['Default'] : '')),
					);
				}
			}
		}
	}
	$variables = array(
    'header' => array(
	array(
        'data' => t('Data'),
	),
	array(
        'data' => t('Mokėjimo tipas')
	),
	array(
        'data' => t('Suma'),
        'class' => array('text-right'),
	),
	array(
        'data' => t('Balansas'),
        'class' => array('text-right'),
	),
	array(
        'data' => t('Būsena')
	),
	array(
        'data' => t('Pastabos')
	),
	),
    'rows' => $rows,
    'attributes' => array(),
    'caption' => NULL,
    'colgroups' => array(),
    'sticky' => TRUE,
    'empty' => t('Įrašų nėra'),
	);

	$html .= theme('table', $variables);
    drupal_set_title(t('Sąskaitos istorija'));
	return theme('account_page', array('title' => t('Sąskaitos istorija'), 'content' => $html));
}

function account_page_info(){
	global $user; $account = clone $user;
	drupal_set_title(t('Mano erdvė'));
	$html = '';
	include_once drupal_get_path('module', 'user') . '/user.pages.inc';
	$form = drupal_get_form('user_profile_form', $account);
	unset($form['locale']);
	unset($form['timezone']);

	$form['group_left'] = array(
    '#tree' => false,
	//'#prefix' => '<div class="left" style="margin-right: 10px;">',
	//'#suffix' => '</div>',
    '#weight' => -50,
	);
	unset($form['data']['country']['#ajax']);
	$form['data']['betcenter']['#access'] = false;

	/*$form['group_left']['text'] = array(
    '#weight' => -20,
    '#markup' => t('Šiame puslapyje jūs galite pakeisti savo duomenis.').'<br/><br/>',
	);*/

	$form['group_left']['account'] = $form['account'];
	$form['group_left']['personal_info'] = $form['personal_info'];
	$form['group_left']['address_info'] = $form['address_info'];
	$form['group_left']['topsport_info'] = $form['topsport_info'];
	$form['group_left']['confirm_info'] = $form['confirm_info'];
	//	$form['group_left']['submit_action'] = $form['submit_action'];
	unset($form['actions']);
   
    $form['group_left']['confirm_info']['pin']['value']['#description'] = t('Pakeitimai patvirtinami įvedus esamą slaptažodį ir PIN numerį iš Jūsų PIN kodų kortelės.') . ' <br>'.
                t('Neturite PIN kodų kortelės?') .' '. l('Spausti čia', 'account/info/pins');
	

    $form['group_left']['actions'] = array(
      '#type' => 'fieldset',
      '#weight' => 999,
    );
    
	$form['group_left']['actions']['submit_action'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 999,
      '#attributes' => array('class' => array('btn-primary'))
	);
	unset($form['personal_info'], $form['address_info'], $form['topsport_info'], $form['confirm_info'], $form['terms_of_use'], $form['account'], $form['submit_action'], $form['text_required']);

	$html .= drupal_render($form);
    drupal_set_title(t('Mano duomenys'));
	return theme('account_page', array('title' => t('Mano duomenys'), 'content' => $html));
}

function account_page_getpins(){
	global $user; $account = clone $user;
	drupal_set_title(t('Mano erdvė'));
	$html = '';
	$form = drupal_get_form('account_get_pins_form', $account);
	$html .= render($form);
    drupal_set_title(t('PIN kodų kortelės gavimas'));
	return theme('account_page', array('title' => t('PIN kodų kortelės gavimas'), 'content' => $html));
}
