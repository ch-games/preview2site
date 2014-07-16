<?php
define('BETGAMESTV_SECRET_KEY', '3HFCB-8ZUV4-UU4X2-DX1WP');
define('BGT_oauth_ID', 1);
define('BGT_CONSUMER_secret', "I_![a;8P9KeHtRxMi@m8]5ppd$ qc-hCoA'}n!Gjt}gCfXUpZblk5\z=j8|~ t?x");
/**
 * This period to check the xml packet timestamp less than 10 seconds age
 */
define('BGT_PACKET_TTL', 10);
/**
 * This is token lifetime age if he will not be updated, 
 * that mean user are offline and need stop access to his all operations.
 */
define('BGT_TOKEN_TTL', 60); 

/**
 * Process all request from BetGamesTV server
 */
function betgamestv_api_service(){
    drupal_add_http_header('Content-Type', 'text/xml');
	drupal_add_http_header('charset', 'utf-8');    
    if ( $_SERVER['REQUEST_METHOD'] === 'POST'){        
        $reguest = file_get_contents('php://input');
        $answer = _betgamestv_api_process($reguest);        
        echo $answer->asXML();
        global $timers;
        $execution = microtime(true) - (isset($timers['page']['start'])?  $timers['page']['start'] : 0) ;
        watchdog('betgamestv', 'Passed request from partner: <br><pre>!request</pre><br><br> Response:<br><pre>!response</pre> <pre>!performance</pre>', 
                array('!request' => print_r($reguest, TRUE), '!response' => print_r($answer, TRUE), '!performance' => print_r(array('memory' => (memory_get_peak_usage(true)/1024/1024) . ' mb', 'execution' => $execution . ' seconds'), TRUE)));
    } else {
        watchdog('betgamestv', 'REQUEST_METHOD was not POST');
    }     
}

/**
 * Impelmented autoload functions:
 * - ping
 * - get_account_details
 * - get_balance
 * - money_deposit
 * - money_withdraw
 * - ping
 * - refresh_token
 * 
 * @param type $xml
 * @return type
 */
function _betgamestv_api_process($xml){
    $xml_obj = new SimpleXMLElement($xml);    
    if(_betgamestv_api_IsValid($xml_obj)){        
        $function = __FUNCTION__.'_'.$xml_obj->method;
        if(function_exists($function)){
            $xml_obj = $function($xml_obj);
            $xml_obj->time      = time();
            $xml_obj->signature = _betgamestv_api_sign($xml_obj);            
        } else {
            $xml_obj->error_code = 2;
            $xml_obj->error_text = 'Sent "method": "'.$xml_obj->method.'" does not implemented yeat!';
        }
    } else {
        $xml_obj->error_code = 1;
        $xml_obj->error_text = 'Sent packet was invalid!';
    }
    return $xml_obj;
}
/**
 * Required tags in XML object:
 * - method
 * - token
 * - params 
 * @param type $object
 * @return boolean
 */
function _betgamestv_api_IsValid(SimpleXMLElement $object){
    $object_cp = clone $object;
    $signature = '';
    unset($object_cp->params, $object_cp->signature);     
    foreach ($object_cp as $key => $value ){ $signature .= $key.$value; }
    if(count($object->params->children()) > 0) {
        foreach ($object->params->children() as $key => $value) { $signature .= $key.$value; }   
    }
    $signature .= BETGAMESTV_SECRET_KEY;    
    if($object->time + BGT_PACKET_TTL > time()){
        if(md5($signature) == $object->signature){
            $avoid_token = array('ping', 'transaction_bet_payout');
            if((in_array($object->method, $avoid_token) && $object->token == '-') || _betgamestv_api_IsValidToken($object->token)) {
                return true;
            } else {                
                watchdog('betgamestv', 'Invalid token!', array(), WATCHDOG_ERROR);
            }                                 
        } else {
            watchdog('betgamestv', 'Invalid signature!', array(), WATCHDOG_ERROR);
        }
    } else {
        watchdog('betgamestv', 'Invalid packet! old: :packet_age sec.', array(':packet_age' => (time() - $object->time)), WATCHDOG_ERROR);
    }
    return false;
}

function _betgamestv_api_sign(SimpleXMLElement $object){
    $signature = '';
    $object_cp = clone $object;
    unset($object_cp->params, $object_cp->signature);  
    foreach ($object_cp as $key => $value ){ $signature .= $key.$value; }
    if(count($object->params->children()) > 0){ 
        foreach ($object->params->children() as $key => $value) { $signature .= $key.$value; }        
    }
    return md5($signature.BETGAMESTV_SECRET_KEY);  
}

/**
 * Let patners know about that your servise is alive
 * @param SimpleXMLElement $object
 * @return \SimpleXMLElement
 */
function _betgamestv_api_process_ping(SimpleXMLElement $object){
    $object->addChild('success',    1);
    $object->addChild('error_code', 0);
    $object->addChild('error_text', '');
    return $object;
}

/**
 * Partners can request new token for security purposes...
 * @param SimpleXMLElement $object
 * @return \SimpleXMLElement
 */
function _betgamestv_api_process_refresh_token(SimpleXMLElement $object){
    $is_valid_token = db_update('consumer_oauth')
        ->fields(array('ttl' => time()))
        ->condition('token', $object->token)
        ->execute();
    if($object->method == 'refresh_token') {
        if($is_valid_token){
            $object->success    = 1;
            $object->error_code = 0;
            $object->error_text = '';
        }else {
            $object->error_code = 22;
            $object->error_text = 'Seems like the requested access_token was removed or invalid';
        }
        
    } 
    return $object;
}

function _betgamestv_api_process_request_new_token(SimpleXMLElement $object){
    $new_token = _betgamestv_api_renewToken($object->token);
    if(strlen($new_token) > 0 && $new_token != $object->token){
        $object->params->new_token = $new_token;
        $object->success    = 1;
        $object->error_code = 0;
        $object->error_text = '';
    } else {
        $object->error_code = 9;
        $object->error_text = 'Problems with new token request!';
    }    
    return $object;
}

/**
 * Providing summarized customer data only by TOKEN!
 * Token logic is secure protect data leak from 3 parthy, 
 * Service can work with customer when he will visit loteries page and are loged in...  
 * @param SimpleXMLElement $object
 * @return type
 */
function _betgamestv_api_process_get_account_details(SimpleXMLElement $object){    
    $user = _betgamestv_api_getUserByToken($object->token);
    if($user->uid > 0 && $user->data['tsuid'] > 0){        
        $object->params->addChild('user_id',  _betgamestv_api_getTSUID2ConsumerUID($user->data['tsuid']));
        $object->params->addChild('username', $user->name);
        $object->params->addChild('currency', $user->data['currency']);
        $object->params->addChild('info',     '-');
        $object->success    = 1;
        $object->error_code = 0;
        $object->error_text = '';
        $object = _betgamestv_api_process_refresh_token($object);
    } else {
        $object->error_code = 4;
        $object->error_text = 'Problems with to provide user data';
    }   
    return $object;
}

/**
 * Provide existing customer main fund 
 * @param SimpleXMLElement $object
 * @return type
 */
function _betgamestv_api_process_get_balance(SimpleXMLElement $object){    
    $user = _betgamestv_api_getUserByToken($object->token);
    if($user->uid > 0 && $user->data['tsuid'] > 0){    
        $userData = topsport_searchClientUser($user->data['tsuid']);
        if(isset($userData['Money']['Balance']) && $userData['Money']['Balance'] >= 0){
            $object->params->balance = $userData['Money']['Balance']*100;
            $object->success    = 1;
            $object->error_code = 0;
            $object->error_text = '';
            $object = _betgamestv_api_process_refresh_token($object);
        } else {
            $object->error_code = 23;
            $object->error_text = 'Problems with to provide user data';
        }        
    } else {
        $object->error_code = 4;
        $object->error_text = 'Invalid params';
    }   
    return $object;
}

function _betgamestv_api_process_transaction_bet_payin(SimpleXMLElement $object){
    $user = _betgamestv_api_getUserByToken($object->token);
    if($user->uid > 0 && $user->data['tsuid'] > 0){
        $params_count = count($object->params->children());
        $params = &$object->params->children();
        if($params_count == 5){
            if($params->amount > 0 && $params->transaction_id > 0 && $params->bet_id > 0 && ($params->retrying == 0 || $params->retrying == 1)){
                $_operation = topsport_saveBetGamesTVSharedWallet(
                            $user->data['tsuid'], 
                            ($params->amount/100) * -1,
                            $params->currency,
                            $params->bet_id
                        );
                if(isset($_operation['Code'])) {
                    unset($params);                    
                    switch ($_operation['Code']) {
                        case 0:
                            $object->params->balance_after = userinfo_get_balance($user->uid, true)*100;
                            $object->params->already_processed = 0;
                            $object->success    = 1;
                            $object->error_code = 0;
                            $object->error_text = '';                            
                            $object = _betgamestv_api_process_refresh_token($object);
                            _betgamestv_setbalanceUpdate($user->data['tsuid']);
                            break;
                        case 10: // TODO: be sure that this code in back-end means was done before
                            $object->params->balance_after = userinfo_get_balance($user->uid, true)*100;
                            $object->params->already_processed = 1;
                            $object->success    = 1;
                            $object->error_code = 0;
                            $object->error_text = '';  
                            $object = _betgamestv_api_process_refresh_token($object);
                            _betgamestv_setbalanceUpdate($user->data['tsuid']);
                            break;                        
                        default:
                            $object->error_code = 100 . $_operation['Code'];
                            $object->error_text = $_operation['Description']['LT']; 
                            break;
                    }                    
                } else {
                    $object->error_code = 403;
                    $object->error_text = 'Can not reach at this moment back-end server, please rollback operations and try again latter...'; 
                }
            } else {
                $object->error_code = 6;
                $object->error_text = 'Sent params are invalid, please check params.'; 
            }
        } else {
            $object->error_code = 5;
            $object->error_text = 'Sent params count invalid: '. $params_count;
        }        
    } else {
        $object->error_code = 4;
        $object->error_text = 'Problems with to provide user data';
    }   
    return $object;
}

function _betgamestv_api_process_transaction_bet_payout(SimpleXMLElement $object){
    if($object->token == '-'){
        $user = _betgamestv_api_user_load_by_consumer($object->params->player_id);
        if($user == false){            
            $object->error_code = 20;
            $object->error_text = 'Was not found registered user. '. print_r($user, true);
            return $object;
        }
    } else {
        $user = _betgamestv_api_getUserByToken($object->token);
    }
    
    if($user->uid > 0 && $user->data['tsuid'] > 0){
        $params_count = count($object->params->children());
        $params = &$object->params->children();
        if($params_count == 5 || $params_count == 6){
            if($params->amount >= 0 && $params->transaction_id > 0 && $params->bet_id > 0 && ($params->retrying == 0 || $params->retrying == 1)){
                $_operation = topsport_saveBetGamesTVSharedWallet(
                            $user->data['tsuid'], 
                            ($params->amount/100),
                            $params->currency,
                            $params->bet_id
                        );
                if(isset($_operation['Code'])) {
                    unset($params);                    
                    switch ($_operation['Code']) {
                        case 0:
                            $object->params->balance_after = userinfo_get_balance($user->uid, true)*100;
                            $object->params->already_processed = 0;
                            $object->success    = 1;
                            $object->error_code = 0;
                            $object->error_text = '';                            
                            $object = _betgamestv_api_process_refresh_token($object);
                            _betgamestv_setbalanceUpdate($user->data['tsuid']);
                            break;
                        case 8: // TODO: be sure that this code in back-end means was done before
                            $object->params->balance_after = userinfo_get_balance($user->uid, true)*100;
                            $object->params->already_processed = 1;
                            $object->success    = 1;
                            $object->error_code = 0;
                            $object->error_text = '';  
                            $object = _betgamestv_api_process_refresh_token($object);
                            _betgamestv_setbalanceUpdate($user->data['tsuid']);
                            break;                       
                        default:
                            $object->error_code = 100 . $_operation['Code'];
                            $object->error_text = $_operation['Description']['LT']; 
                            break;
                    }                    
                } else {
                    $object->error_code = 403;
                    $object->error_text = 'Can not reach at this moment back-end server, please rollback operations and try again latter...'; 
                }
            } else {
                $object->error_code = 6;
                $object->error_text = 'Sent params are invalid, please check params.'; 
            }
        } else {
            $object->error_code = 5;
            $object->error_text = 'Sent params count invalid: '. $params_count;
        }        
    } else {
        $object->error_code = 4;
        $object->error_text = 'Problems with to provide user data';
    }   
    return $object;
}

function _betgamestv_api_getToken($uid){ 
    $access_token = '-';
    db_delete('consumer_oauth')->condition('ttl', time()-BGT_TOKEN_TTL, '<')->execute();
    $token = db_select('consumer_oauth', 'coa')->fields('coa')->condition('uid', $uid)->execute();    
    if($token->rowCount() == 0){        
        $_rez = db_insert('consumer_oauth')->fields(array(
          'consumer_id' => BGT_oauth_ID,         
          'uid'         => $uid,
          'created'     => time(),
          'ttl'         => time(),
          'token'       => _betgamestv_api_generateTokenString('', $uid),    
        ))->execute();        
        db_update('consumer_oauth')
            ->fields(array('token' => _betgamestv_api_generateTokenString($_rez, $uid), 'ttl' => time()))
            ->condition('id', $_rez)
            ->execute();
        $_token       = db_select('consumer_oauth', 'coa')
                            ->fields('coa', array('token'))
                            ->condition('uid', $uid)
                            ->execute()
                            ->fetchObject();
        $access_token = $_token->token;         
    } else if($token->rowCount() == 1) {
        $_obj_token = $token->fetchObject();
        if(strlen($_obj_token->token) > 0){
               $access_token = $_obj_token->token;
        }      
    }
    return $access_token;
}

function _betgamestv_api_renewToken($token){
    $old_token = db_select('consumer_oauth', 'coa')
                    ->fields('coa')
                    ->condition('token', $token)
                    ->execute()
                    ->fetchObject();   
    db_delete('consumer_oauth')->condition('token', $old_token->token)->execute();
    $l_token_id = db_insert('consumer_oauth')->fields(array(
        'consumer_id' => BGT_oauth_ID,         
        'uid'         => $old_token->uid,
        'created'     => time(),
        'ttl'         => time(),
        'token'       => _betgamestv_api_generateTokenString('', $old_token->uid),    
    ))->execute();
    db_update('consumer_oauth')
        ->fields(array('token' => _betgamestv_api_generateTokenString($l_token_id, $old_token->uid), 'ttl' => time()))
        ->condition('id', $l_token_id)
        ->execute();
    return db_select('consumer_oauth', 'coa')
            ->fields('coa', array('token'))
            ->condition('uid', $old_token->uid)
            ->execute()
            ->fetchObject()->token;
}

function _betgamestv_api_generateTokenString($token_id, $uid){
    return hash ( 'sha256' , BGT_oauth_ID.BGT_CONSUMER_secret . (time() + BGT_PACKET_TTL) . $token_id.$uid);
}

function _betgamestv_api_IsValidToken($token){    
    $_obj = db_select('consumer_oauth', 'oauth')
                ->fields('oauth', array('token'))
                ->condition('token' , $token)
                ->condition('ttl', time() - BGT_TOKEN_TTL, '>=')
                ->execute();
    return ($_obj->rowCount() == 1) ? true : false;
} 

function _betgamestv_api_getUserByToken($token){
    $l_token = db_select('consumer_oauth', 'oauth')
            ->fields('oauth', array('uid'))
            ->condition('token' , $token)
            ->execute();
    if($l_token->rowCount() == 1){
        $l_token = $l_token->fetchObject();
        return user_load($l_token->uid);
    }
    return false;
}

function pass_gen($length = 64) {
    $pass = '';
    srand((float) microtime() * 10000000);
    for ($i = 0; $i < $length; $i++) {
        $pass .= chr(rand(32, 126));
    }
    return $pass;
}

function _betgamestv_api_DestroyToken($uid) {
    db_delete('consumer_oauth')->condition('uid', $uid)->execute();
}

function _betgamestv_api_getTSUID2ConsumerUID($tsuid) {
   $consumer_user = db_select('consumer_users_map', 'oauth')
           ->fields('oauth', array('id'))
           ->condition('consumer_id' , BGT_oauth_ID)
           ->condition('tsuid' , $tsuid)
           ->execute();
    if($consumer_user->rowCount() == 0){
        $consumer_user_id = db_insert('consumer_users_map')
            ->fields(array(
              'consumer_id' => BGT_oauth_ID,
              'tsuid'       => $tsuid
            ))->execute();
    } else {
        $consumer_user_id = $consumer_user->fetchObject()->id;
    } 
    return $consumer_user_id;
}

function _betgamestv_api_getConsumerUID2TSUID($consumer_uid) {
    $user = db_select('consumer_users_map', 'u')
            ->fields('u', array('tsuid'))
            ->condition('consumer_id' , BGT_oauth_ID)
            ->condition('id' , $consumer_uid)
            ->execute();
    if($user->rowCount() == 1){
        $consumer_user_id = $user->fetchObject()->tsuid;
    } else {
        $consumer_user_id = -1;
    }
    return $consumer_user_id;
}

function _betgamestv_api_user_load_by_consumer($consumer_uid) {
    $tsuid = _betgamestv_api_getConsumerUID2TSUID($consumer_uid);    
    if($tsuid > 0){
        $_user = db_select('users', 'u')
            ->fields('u')
            ->where('data LIKE \'%s:5:"tsuid";s:' . strlen($tsuid) . ':"' . db_like($tsuid) . '"%\'')
            ->range(0, 1)
            ->execute();
        if($_user->rowCount() == 1){
            $user = $_user->fetchObject();
            $user->data = unserialize($user->data);
        } else {
           $user = false;
        }
    } else {
        $user = false;
    }    
    return $user;
}

function topsport_saveBetGamesTVSharedWallet($tsuid, $amount, $currency, $bet_id){
    $response = topsport_http_request(
	  "saveBetgTvSharedWalletXML.jsp", 
      array(
      		'user_id'  => $tsuid, 
      		'amount'   => ($amount == 0) ? $amount : account_currency_api_convert($currency, 'LTL', $amount), 
      		'currency' => 'LTL', 
      		'bet_id' => $bet_id 
      )
	);
	$result = sx_array(simplexml_load_string($response));
	return $result;   
}

function _betgamestv_setbalanceUpdate($ts_uid){
    $cid = 'account_user_'.$ts_uid.'_force_balance_update';
    cache_set($cid, true);
}