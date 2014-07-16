<?php

/**
 * Live sportsbook server URL
 * @var string
 * STAGES:
 * 	TESTING : http://front.dev.topsport.lt/fe_home
 *  STAGE   : http://sportsstage.topsport.lt/
 *  LIVE    : https://front.topsport.lt/fe_home
 */
define('OM_SERVER', 		'//front.topsport.lt/fe_home');
define('OM_BACKEND_SERVER', 'http://sportsro.bettopsport.com/');
/**
 * TOP SPORT ID into OddsMatrix API service
 * @var string
 */
define('OM_OPERATOR_ID', '44topsport');

/**
 * Odds Matrix side operator ID
 * @var integer
 */
define('OM_OPERATOR_NUMBER', 44);

/**
 * HTTP Auth user name
 * @var string
 */
define('OM_ACC_USER', 'Topsport');

/**
 * HTTP Auth password
 * @var string
 */
define('OM_ACC_PASS', 't0spert');


/**
 * Request function initiate connection via http to server
 * @param string $url
 * @param array $data
 * @param string $method
 * @param boolean $log
 */
function livesportsbook_OM_request( $url, array $data = null, $method = 'GET', $log = TRUE ) {

	if (isset($data)) {
		$data = http_build_query( $data, '','&' );
	}

	$options = array(
		'headers' => array( 'Content-Type' => 'application/x-www-form-urlencoded', 'authorization' => "Basic " . base64_encode( OM_ACC_USER . ":" . OM_ACC_PASS )),
		'method'  => $method,
		'data'    =>  $data,
		'timeout' => 60,
	);

	$respond = drupal_http_request( OM_BACKEND_SERVER . $url, $options);

	if ( !$respond OR $respond->code <> 200  ){
		$message = 'Requested URL: <i>' . OM_BACKEND_SERVER . $url .'</i><br /><pre>' . print_r($respond, true) . '</pre>';
		watchdog('livesportsbook_OM_request', '!message', array( '!message' => $message ), WATCHDOG_ERROR );
		return false;
	}

	return $respond->data;
}

/**
 * Register customer on Odds Matrix live betting system
 * @return boolean
 */
function livesportsbook_registerPlayer ( &$user ) {
	if ( !isset( $user->uid) ) return false;

	$sex = ( isset($user->data['sex']) && $user->data['sex'] == 1 ) ? 'm' : 'f';
	$_timestamp  = new DateTime( 'now', new DateTimeZone( empty($user->timezone) ? 'Europe/Vilnius' : $user->timezone ) );
	$time_offset = $_timestamp->format('O');

    $user->data['currency'] = isset($user->data['currency']) ? $user->data['currency'] : 'LTL';
	$query = array(
		'playerfrom' => OM_OPERATOR_ID,
		'xmlString'  => "<UserEntity><userName>{$user->data['tsuid']}</userName><email>{$user->mail}</email><isEnabled>1</isEnabled><familyName>{$user->data['surname']}</familyName><givenName>{$user->data['firstname']}</givenName><gmtTimeZoneOffset>{$time_offset}</gmtTimeZoneOffset><addressLine1>{$user->data['street']}</addressLine1><city>{$user->data['city']}</city><zipCode>{$user->data['zip']}</zipCode><country>{$user->data['country']}</country><preferredCurrency>{$user->data['currency']}</preferredCurrency><birthDate>{$user->data['birthday']}</birthDate><gender>{$sex}</gender><currentSession>{$user->sid}</currentSession></UserEntity>"
	);

	$query = http_build_query( $query, '', '&');
	$result = trim( livesportsbook_OM_request( 'partnerapi1/registerPlayer?' . $query ) );

	if ( $result === 'register succeeded' || $result === 'user exists' ){
		$user->data['om_registered'] = true;
		$user->data['om_currency']   = $user->data['currency'];
		user_save( $user );
		// make log fo user
		account_log($user->uid, 'OM Register Player API', 'Vartotojas <i>%name</i> sėkmingai užregistruotas OddsMatrix sistemoje.', array( '%name' => $user->name ));
		return true;
	} else {
		$message = '<b>Om request:</b><br /><pre>' . print_r($query, true) . '</pre><br /><br /><b>OM respond</b>: '.$result;
		watchdog('OM_register_fail', '!message', array( '!message' => $message ), WATCHDOG_ERROR );
	}

	return false;
}

/**
 * Client goes out from live service
 * @return response body
 */
function livesportsbook_LogoutPlayer( $sid, $name ){
	$query = array(
		'currentSession' => $sid,
		'username' 		 => $name
	);

	$query  = http_build_query( $query, '', '&');
	$result = livesportsbook_OM_request( 'partnerapi1/customerLogout.do?' . $query );

	watchdog('customerLogout.do', '@user made log out <b>response from OM:</b><br/><pre>@response</pre>', array(
		'@user' => $name, 
		'@response' => print_r($result, true)
	), WATCHDOG_INFO);

	return $result;
}


/**
 * Atlieka pagrindinę balanso pervedimo operaciją
 */
function livesportsbook_walletTransfer( &$user, $amount ) {
	// init OM wallet transfer
	$query = array(
		'username' => $user->data['tsuid'] 				
	);
	$init_query 		  = http_build_query( $query, '', '&');
	$init_transfer_result = livesportsbook_OM_request( 'partnerapi1/walletTransfer.do?' . $init_query );
	$init_transfer_result = sx_array( @simplexml_load_string( $init_transfer_result) );



	// brake all operation if something went wrong and do dblog
	if ( !isset($init_transfer_result['walletTransferId']) OR isset( $init_transfer_result['errorCode'] ) ) {
		$message = '<strong>CLIENT REQUEST</strong><br />' . print_r( $query, true ) . '<br /><strong>Odds Matrics RESPONDE</strong><br /><pre>'. print_r( $init_transfer_result, true) . '</pre>';
		watchdog('walletTransfer.do', '!message', array('!message' => $message), WATCHDOG_ERROR);
		drupal_set_message( t('Nepavyko pervesti nurodytos sumos, bandykite vėliau.'), 'error');
		return false;
	}
	$init_transfer_result['status'] = 'TRANSFER_INITIATED';

	// put into transfer db log
	db_insert('livesportsbook_wallet')
	->fields(array(
			'wid' 			=> $init_transfer_result['walletTransferId'],
			'amount' 		=> $amount,
			'currency' 		=> $init_transfer_result['currency'],
			'uid' 			=> $user->uid,
			'created' 		=> date( 'Y-m-d H:i:s', time() ),
			'transfer_time' => null,
			'status' 		=> $init_transfer_result['status'],
			'message' 		=> null,
	))
	->execute();
	// update new request params
	$query['walletTransferId'] = $init_transfer_result['walletTransferId'];
	// final wallet transfer
	$query = array_merge( $query, array(
			'amount'    => $amount,
			'currency'  => $init_transfer_result['currency'],
			'isRealDep' => 'true',
			'use1stDep' => 'true',			
	));
	// sending request to TOPSPORT make payin/payout operation
	$topsport_params = array(
		'wid' 	    => $init_transfer_result['walletTransferId'],
		'amount' 	=> account_currency_api_convert( $init_transfer_result['currency'] , 'LTL', $amount, 6),
		'currency' 	=> 'LTL',  // TOPSPORT accept only LTL currency $init_transfer_result['currency']
		'status' 	=> $init_transfer_result['status'],
		'user_id'   => $user->data['tsuid'],  
	);
	// make reservation money
	if( !topsport_save_wallet_amount( $topsport_params ) ) {
		drupal_set_message( t('Nepavyko pervesti nurodytos sumos, bandykite vėliau.'), 'error');
		return false;
	}

	// if was okey with TOPSPORT we connecting to OM and sending confirmation to whey allow reserwed amount for client.
	$final_query 		   = http_build_query( $query, '', '&');
	$final_transfer_result = livesportsbook_OM_request( 'partnerapi1/walletTransfer.do?' . $final_query );
	$data 		           = sx_array( @simplexml_load_string( $final_transfer_result ) );

	if ( ! isset( $data['status'] ) ){
		$message = 'Fatal error, STATUS xml tag not found... <br /><strong>CLIENT REQUEST</strong><br />' . print_r( $query, true ) . '<br /><strong>Odds Matrics RESPONDE</strong><br /><pre>'. print_r($data, true) . '</pre>';
		watchdog('livesportsbook_walletTransfer', '!message', array('!message' => $message), WATCHDOG_ERROR);
		drupal_set_message( t('Nepavyko pervesti nurodytos sumos, bandykite vėliau.'), 'error');
		return false;
	} else {
		$topsport_params['status'] = $data['status'];
		if ( !topsport_save_wallet_amount( $topsport_params ) ){
			drupal_set_message( t('Jūsų nurodytos sumos nepavyko perverti, bandykite vėliau.'), 'error');
			return false;
		}else {
			db_update('livesportsbook_wallet')->fields( array(
				'transfer_time' => date( 'Y-m-d H:i:s', time() ),
				'status' 		=> $data['status'],
				'message'       => print_r( $data['message'], true ),
				'error_code'    => 0
			))
			->condition('wid', $data['walletTransferId'], '=')
			->execute();
		}
		if ( $data['status'] == 'TRANSFER_COMPLETE' ) {
			if ( $amount > 0) {
				drupal_set_message( t('Jūsų nurodyta suma: @amount @currency, buvo sėkingai pervesta į "lažybos gyvai" balansą.', array(
					'@amount'   => betslip_number_format($amount), 
					'@currency' => $init_transfer_result['currency'] )) 
				);
			} else {
				drupal_set_message( t('Jūsų nurodyta suma: @amount @currency, buvo sėkingai pervesta atgal į Jūsų balansą.', array(
					'@amount'   => betslip_number_format(-$amount), 
					'@currency' => $init_transfer_result['currency'] )) 
				);
			}
		} elseif ( $data['status'] == 'TRANSFER_PENDING' ) {
			drupal_set_message(t('Jūsų operacija nurodyta suma nurašyta, tačiau Jūs viršijote pinigų pervedimo limitą 2,300EUR. Pervedimo operacijos į lažybas gyvai yra pristabdyta 24 valandoms.'));

		} elseif ( $data['status'] == 'TRANSFER_ABORTED' ) {
			drupal_set_message(t('Jūsų pavedimas buvo atšauktas.'));

		} elseif ( $data['status'] == 'TRANSFER_QUEUED' ) {
			drupal_set_message(t('Dėl perdidelio sistemos apkrovimo Jūsų operacija įrašyta į opeacijų eilę, kurios bus artimiausiu metu bandomos vykdyti.'));
		} elseif ( $data['status'] == 'TRANSFER_INITIATED') {
				
		}
	}
}

/**
 * Getting specified customer balance data by TRANSFER_ID
 * @param unknown_type $transfer_id
 * @return Ambiguous
 */
function livesportsbook_walletTransferInfo( $transfer_id ) {
	$query = array(
		'walletTransferId' => 	$transfer_id			
	);

	$query   = http_build_query( $query, '', '&');
	$results = livesportsbook_OM_request( 'partnerapi1/walletTransferInfo.do?' . $query );

	return $results;
}

/**
 * Get user Live betting wallet amount
 * @return object
 */
function livesportsbook_walletBalance ( &$user ) {

	$query = array(
		'username' => $user->data['tsuid'] 				
	);
	$query   = http_build_query($query, '', '&');
	$results = livesportsbook_OM_request( 'partnerapi1/walletBalance.do?' . $query );

	if ( strpos($results, 'amount') === FALSE ){
		$message = 'Username <b>'. $user->name . '</b> tried get current balance and get message: <br /><i>'
		.print_r($results, true) . '</i>';
		watchdog('GETOMwalletBalance', '!message', array('!message' => $message), WATCHDOG_ERROR);
		return false;
	}
	$results = @simplexml_load_string( $results );
	return $results;
}

/**
 * Get list of active payments
 * and sync with Odds matrix and TOPSPORT database and do changes if its necessary...
 */
function get_opened_wallet_transfers() {
	//xdebug_print_function_stack( 'Your own message' );
	$items_count = 0;
	$operation_count = 0;
	$transfer_data = db_query(
		"SELECT
			w.wid,
			w.uid,
			w.status,
			u.data
		FROM
			{livesportsbook_wallet} w
		LEFT JOIN {users} u ON u.uid = w.uid
		WHERE
			w.status IN(
				'TRANSFER_PENDING',
				'TRANSFER_QUEUED',
				'TRANSFER_INITIATED')
			AND error_code IS NULL		
		LIMIT 100"
		);

		if ( $transfer_data ) {
			foreach ($transfer_data as $transfer){
				$transfer->data = unserialize($transfer->data);
				$result = livesportsbook_walletTransferInfo($transfer->wid);
				$result = @simplexml_load_string($result);
				if ($result){
					if ( ($transfer->status != (string)$result->walletTransfer->status) && (float)$result->walletTransfer->amount <> 0.0 ) {
						$topsport_save_wallet_amount_status = topsport_save_wallet_amount(array(
						'wid' 	    => $transfer->wid,
						'amount' 	=> account_currency_api_convert( (string)$result->walletTransfer->currency , 'LTL', (float)$result->walletTransfer->amount),
						'currency' 	=> 'LTL',  // TOPSPORT accept only LTL currency $init_transfer_result['currency']
						'status' 	=> (string)$result->walletTransfer->status,
						'user_id'   => $transfer->data['tsuid'],  
						));
						if ( $topsport_save_wallet_amount_status ){
							$operation_count++;
							db_update('livesportsbook_wallet')
							->fields(array(
						  'status'     => (string)$result->walletTransfer->status,
						  'error_code' => 0,
							))
							->condition('wid', $transfer->wid)
							->execute();
						}
					}elseif ( (float)$result->walletTransfer->amount == 0.0 ) {
						db_update('livesportsbook_wallet')
						->fields(array(
					  'status'     => (string)$result->walletTransfer->status,
					  'error_code' => 0,
						))
						->condition('wid', $transfer->wid)
						->execute();
					}
				}
				$items_count++;
			}
		}
		if($operation_count > 0){
	 	watchdog('saveOMwalletOperation', 'Found <b>!count_items</b> Closed <b>!count</b> operations', array('!count' => $operation_count, '!count_items' => $items_count), WATCHDOG_INFO);
		}
		echo 'done ';
}

/**
 * Control web operation remontly
 * @return boolean
 */
function topsport_WebOperationsAreAllowed(){
	return (bool)variable_get('topsport_WebOperationsAreAllowed', true);
}

/**
 * This operation connects to TS server geting permission make main wallet changes.
 * @param array $params
 * @todo: baigti komunikavimo logika
 */
function topsport_save_wallet_amount( array $params ) {
	$data     = http_build_query($params, '', '&');
	$response = topsport_http_request( "saveOddsMatrixWalletXML.jsp", $data );
	$result   = sx_array( @simplexml_load_string($response) );

	// if we pass Code = 0 its succeded operation sign
	if ( isset($result['Code']) &&  $result['Code'] == 0 ){
		return true;
	}
	if ( $result['Code'] == 10 ){
		variable_set('topsport_WebOperationsAreAllowed', false);
	}else if ( $result['Code'] == 8 ) {
		drupal_set_message( t('Jūsų balansas nepakankamas, prašome papildyti <a href="/mano-paskyra/saskaitos-papildymas">balansą</a>.'), 'error');
	}

	global $user;

	$accountData = array(
        'Kliento kortelės Nr.:' => $user->data['UserCardNumber'],
        'Vardas, pavardė:'  	=> $user->data['firstname'].' '.$user->data['middlename'].' '.$user->data['surname'],
        'Topsport Id:'  		=> $user->data['tsuid'],
        'Topsport.lt ID:' 		=> $user->uid,
        'Nikas:'   				=> $user->name,
        'El. pašto adresas:'  	=> $user->mail,
	);
	/*
	 topsport_sendMail('reserve_amount', $message);*/

	$message = "<br/>".(isset($result['ErrorMessage']) ? $result['ErrorMessage'] : '')."<br/>CLIENT<br/><pre>".print_r($accountData, true).'</pre>'."<br/>POST<br/><pre>".print_r($params, true).'</pre>'."<br/>ANSWER<br/><pre>".print_r($result, true).'</pre>';
	$useraget = $_SERVER['HTTP_USER_AGENT'];
	watchdog('saveOMwalletOperation', 'Client browser: !useragent !message', array('!message' => $message, '!useragent' => $useraget), WATCHDOG_ERROR);

	return false;
}