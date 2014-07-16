<?php

function racingdogs_main_page() {
    //drupal_set_message(t('Atsiprašome šiuo metu paslauga neteikiama.'), 'warning');
    //drupal_goto();
    $title = t('Šunų lenktynės internetu');
    drupal_set_title($title);
    global $user;
    $html = '';

    if (!racingdogs_searchRDWebOperationsAreAllowed()) {
        $html .= t('Lošti šiuo metu negalima. Pamėginkite vėliau');
    } else {
        drupal_add_js(drupal_get_path('module', 'racingdogs') . '/rd_scripts.js');
//		$html .= '
//    <div id="silverlightRequired" style="clear:both; display: none;">
//      <div class="messages error">
//      <h2 class="element-invisible">Įspėjimo žinutė</h2>
//      <ul>
//        <li>'.t('Norint lošti šunų lenktynes Jums reikia kompiuteryje įdiegti <a target="_blank" href="http://www.microsoft.com/getsilverlight"><u>Silver Light</u></a> įskiepį.').'</li>
//        <li>'.t('<a target="_blank" href="http://www.microsoft.com/getsilverlight"><u>Spauskite čia</u></a> norint parsisiųsti ir įsidiegti Jūsų kompiuteryje').'</li>
//        <li>'.t('Atlikus diegimus atidarykite šį puslapį iš naujo ir galėsite pradėti lošti').'</li>
//      </ul>
//      </div>
//    </div>';
        if (isset($user->data['tsuid']) AND $user->data['tsuid']) {
            userinfo_updateUser($user);
            $playForMoney_form = drupal_get_form('racingdogs_playForMoney_form', $user);
            $html .= drupal_render($playForMoney_form);
            $playForFun_form = drupal_get_form('racingdogs_playForFun_form', $user);
            $html .= drupal_render($playForFun_form);
            //$html .= '<div class="clear"><br/></div>';
            if (user_access('access account racindogs')) {
                $html .= '<hr>';
                $html .= racingdogs_drawGamesList($user);
            }
        } else {
            $playForFun_form = drupal_get_form('racingdogs_playForFun_form', $user);
            $html .= drupal_render($playForFun_form);
            $playForMoney_form = drupal_get_form('racingdogs_playForMoney_form', $user);
            $html .= drupal_render($playForMoney_form);
        }
//		$html .= '<h4>'.t('Šunų lenktynių stebėjimo ir statymų sistemos išvaizda').'</h4>
//                <iframe width="100%" height="360" src="//www.youtube.com/embed/yqKsHcR7HGQ?feature=player_detailpage" frameborder="0" allowfullscreen></iframe>';
    }
    return '<div class="panel panel-page"><div class="panel-heading"><h2>' . $title . '</h2></div><div class="panel-body">' . $html . '</div></div>';
}

function racingdogs_game_page($gameId) {
    drupal_set_title(t('Šunų lenktynės internetu'));
    global $user;
    $custid = ((isset($user->data['tsuid']) AND $user->data['tsuid']) ? $user->data['tsuid'] : 0);
    $sessionData = racingdogs_getSessionInfo($gameId, $custid);
    if (!$sessionData) {
        drupal_set_message(t('Lošimas nerastas'), 'error');
        drupal_goto('racingdogs');
        exit;
    }
    drupal_add_js('function resizeRacingDogs(){
    var height = jQuery(\'#racingDogsWindows\').width() * 0.65;
    if(height) jQuery(\'#racingDogsWindows\').height(height);
  }
  jQuery(document).ready(function($){
    resizeRacingDogs();
    $(window).resize(function () { resizeRacingDogs(); });
  });', 'inline');
    return '<div class="panel-body" style="padding:10px;"><iframe src="' . $sessionData->url . '" width="100%" height="500px" frameborder="0" name="racingDogs" id="racingDogsWindows"></iframe>'
            //.'<p class="rd-full-screen-recommend">'.t('Šunų lenktynes rekomenduojame lošti pilname ekrane, tai padaryti spauskite "Per visą ekraną" mygtuką.').'</p>'
            . ($sessionData->fun == 'n' ? '<p><center>' . t('Norint baigti lošimą ir iškarto persivesti šunų lenktynių kreditą spauskite "baigti žaidimą", kitu atveju jūsų @sitename balansas atsinaujins automatiškai 1-5 min intervale.', array('@sitename' => variable_get('site_name'))) . '</center></p>' : '')
            . '<center>' . l(t('Baigti lošimą'), 'racingdogs/finish/' . $gameId, array('attributes' => array('class' => array('btn btn-primary')))) . '</center>' . '
  <div class="racing-dogs-info-game"><p><hr/>
    ' . t('<h4>Statymų sumos:</h4>
    Lažintis galima pasirinkus žetonus: <br/>
    - mėlynas (1 LTL), <br/>
    - raudonas (2 LTL), <br/>
    - žalias (5 LTL), <br/>
    - violetinis (10 LTL)<br/><br/>

    <h4>Statymų tipai:</h4>
    - Kas atbėgs pirmas.<br/>
    - Kas atbėgs poroje pirmas ir antras.<br/><br/>

    <h4>Kaip lošti?</h4>
    1. Pasirinkite artėjantį bėgimą (artėjantis bėgimas lentelėje pažymėtas mėlyname fone), pasirinktas bėgimas, bet dar neprasidėjęs pažymėtas geltonu fonu, vykstantis bėgimas pažymėtas žaliu fonu.<br/>
    2. Statymų lentelė ir instrukcijos:<br/>&nbsp;&nbsp;&nbsp;2.1. Pasirinkite norimą statymo sumą (žetoną, žr. statymų sumų sąrašą);<br/>&nbsp;&nbsp;&nbsp;2.2. Statymo lentelė sudaryta iš 6 eilučių ir 6 stulpelių, kurie pažymėti skaičiais ir skirtingais fonais, atitinkančius šunų skiriamuosius ženklus.<br/>
    &nbsp;&nbsp;&nbsp;2.3. Norint statyti kas atbėgs pirmas reikia horizontalės ir vertikalės sankirtą pasirinkti taip, kad ji būtų istrižainės padėtyje (istrižainė išskirta tamsesniu fonu), sankirtos langelyje yra skaičius (koeficientas), kuris nurodo Jūsų statomos sumos daugiklį sėkmės atveju, tai yra paprastas statymo variantas.<br/>
    &nbsp;&nbsp;&nbsp;2.4. Norint satyti kombinuotą statymą (kas atbėgs pirmas ir antras poroje), reikia suvesti horizontalę ir vertikalę padėtį taip, kad sankirta būtų Jūsų pageidaujamas statymas. Tai yra kombinuotas statymas.<br/>
    &nbsp;&nbsp;&nbsp;2.5. Norint padidinti statymo sumą iki pageidaujamos, jums reikia paspausti ant jau egzistuojančio statymo (žetono esančio statymo lentelėje) ir spausti su pele tiek kartų, kiek reikia norint gauti norimą sumą (maksimali suma už vieną baigtį 10 LTL).<br/>
    &nbsp;&nbsp;&nbsp;2.6. Statymo variantai ir kiekiai nėra ribojami.<br/>
    &nbsp;&nbsp;&nbsp;2.7. Visus statymus galite atlikti iki pasirinkto bėgimo pradžios.<br/>
    3. Laimėtos sumos papildys Jūsų šunų lenktynių balansą (angl. credit);<br/>
    4. Nusprendus baigti žaidimą, Jūsų kreditas (angl. credit) neįtakos Jūsų Top Sport balanso.<br/>') . '
  </p></div></div>';
}

function racingdogs_finish_page($gameId, $check = false, $ajax = 'nojs') {
    drupal_goto('racingdogs');
    exit;





    global $user;

    racingdogs_registerFinishedGames();

    $custid = ((isset($user->data['tsuid']) AND $user->data['tsuid']) ? $user->data['tsuid'] : 0);
    if (!$custid) {
        drupal_goto('racingdogs');
    }

    $sessionData = racingdogs_getSessionInfo($gameId, $custid);
    if (!$sessionData) {
        drupal_set_message(t('Lošimas nerastas'), 'error');
        drupal_goto('racingdogs');
        exit;
    }

    if ($check) {
        switch ($check) {
            case 'check':
                racingdogs_registerSesssionEnd($sessionData);
                if ($ajax != 'ajax') {
                    drupal_goto('racingdogs/finish/' . $gameId);
                }
                $commands = array();
                if ($sessionData->endd) {
                    $commands[] = ajax_command_html('#myProgressBar', '');
                    userinfo_updateUser($user);
                    $commands[] = ajax_command_append('#myProgressBar', '
			        <div class="messages status">
			          ' . t('Jūsų balansas atnaujintas') . '
			        </div>');
                    $commands[] = ajax_command_append('#myProgressBar', '<strong>' . t('Įsinešta suma:') . '</strong> ' . account_currency_convert_string(false, $sessionData->payin, $sessionData->curr) . '<br/>' .
                            '<strong>' . t('Išmokėta suma:') . '</strong> ' . account_currency_convert_string(false, $sessionData->payout, $sessionData->curr) . '<br/>' .
                            '<strong>' . t('Balansas:') . '</strong> ' . account_balance_string() . '<br/>'
                    );
                    $commands[] = ajax_command_invoke('.my-balance a', 'html', array(account_balance_string()));

                    $row = racingdogs_drawGamesList_row($sessionData, $user);
                    $variables = array(
                      'rows' => array($row),
                      'attributes' => array(),
                    );
                    $rowdata = racingdogs_format_rows($variables, false);
                    $commands[] = ajax_command_invoke('#racingdogs-games-table tbody #s_' . $sessionData->sessionid, 'html', array($rowdata));
                    $commands[] = ajax_command_invoke('#racingdogs-games-table tbody #s_' . $sessionData->sessionid, 'removeClass', array('waiting'));
                    $commands[] = ajax_command_invoke('#racingdogs-games-table tbody #s_' . $sessionData->sessionid, 'addClass', $row['class']);
                }
                echo ajax_render($commands);
                exit;
                break;
            default:
                drupal_goto('racingdogs');
                break;
        }
        exit;
    }

    if (!$sessionData->enddt) {
        racingdogs_registerSesssionEnd($sessionData);
    }

    drupal_set_title(t('Šunų lenktynės internetu'));
    $html = '<h3>' . t('Lošimas baigtas') . '</h3>';
    if ($sessionData->fun == 'y') {
        $html .=
                '<p>' . t('Jūs žaidėte pramogos rėžimu, todėl Jūsų išlošti ir/ar pralošti pinigai neįtakos Jūsų @sitename balanso', array('@sitename' => variable_get('site_name'))) . '</p>';
    } else {
        if ($sessionData->enddt) {
            $html .= '<div id="myProgressBar">' . '<strong>' . t('Įsinešta suma:') . '</strong> ' . account_currency_convert_string(false, $sessionData->payin, $sessionData->curr) . '<br/>' .
                    '<strong>' . t('Išmokėta suma:') . '</strong> ' . account_currency_convert_string(false, $sessionData->payin, $sessionData->curr) . '<br/>' .
                    '<strong>' . t('Balansas:') . '</strong> ' . account_balance_string() . '<br/></div>';
        } else {
            $html .= '
      <div id="myProgressBar">
        ' . l(t('Duomenys atnaujinami'), 'racingdogs/finish/' . $gameId . '/check/nojs/', array('attributes' => array('id' => array('update-rd-data'), 'class' => array('no-underline', 'use-ajax')))) . '
        ' . '<div><br/><br/>
          ' . t('Prašome palaukti, vyksta  jūsų balanso atnaujinimas, gali užtrukti iki vienos minutės.') . '<br/>' .
                    t('Jei susidūrėte su problemomis prašome kreiptis <a href="mailto:info@topsport.lt">info@topsport.lt</a> arba telefonu +370 37 409105') . '
        </div>' . '
      </div>';
            drupal_add_js('jQuery(document).ready(function ($){ $("#update-rd-data").trigger("click"); });', 'inline');
        }
    }
    $html .= '<center>' . l(t('Lošti dar kartą'), 'racingdogs', array('attributes' => array('class' => array('form-submit-long')))) . '</center>';

    return $html;
}

function racingdogs_stats_page($sessionid = null, $page = 1, $ajax = 'nojs') {
    global $user;
    $account = clone $user;
    drupal_set_title(t('Mano erdvė'));
    if (is_numeric($sessionid)) {
        return racingdogs_stats_details_page($account, $sessionid);
    } else {
        return racingdogs_drawGamesList($account, $page, ($ajax == 'ajax'));
    }
}

function racingdogs_drawGamesList($account, $page = 1, $ajax = false) {
    global $user;
    $html = '';
    $form_essionsStatsFilter = drupal_get_form('racingdogs_sessionsStatsFilter');
    $form = drupal_render($form_essionsStatsFilter);
    $filterdata = racingdogs_filterData();
    $sessions = racingdogs_getSessionInfo(false, $account->data['tsuid'], $filterdata, $page);

    $headers = array(
      array('data' => t('Pradžia')),
      array('data' => t('Pabaiga')),
      array('data' => t('Įsinešta'), 'class' => array('payin')),
      array('data' => t('Statyta'), 'class' => array('stake')),
      array('data' => t('Laimėta'), 'class' => array('win')),
      array('data' => t('Išmokėta'), 'class' => array('payout')),
      array('data' => ''),
    );
    $rows = array();
    $waiting_sessions = array();
    foreach ($sessions as $sessionData) {
        $rows[] = racingdogs_drawGamesList_row($sessionData, $account, $waiting_sessions);
    }
    $variables = array(
      'rows' => $rows,
      'header' => $headers,
      'empty' => t('Lošimų nėra'),
      'attributes' => array(
        'id' => 'racingdogs-games-table',
        'class' => array('lost-won-colors')
      )
    );
    if (count($waiting_sessions) > 0) {
        drupal_set_message(t('Laukiama bėgimo(-ų) pabaigos ir statymų apdorojimo (trukmė ~1min).'), 'warning');
        drupal_add_js('/front/templates/files/ninjax.js');
        drupal_add_js('
				var closeSess = {sessions: ' . json_encode($waiting_sessions) . '};
				var timeoutid;
				function racingdogs_sessions_cron(){					
    		  		ninjax.work("/front/Racingdogs/close",closeSess);					
                    timeoutid = setTimeout(racingdogs_sessions_cron, 7000);
				}				
				jQuery(document).ready(function() {    				
                  timeoutid = setTimeout(racingdogs_sessions_cron, 2000);
				});', 'inline');
    }
    if ($ajax) {
        $commands = array();
        $commands[] = ajax_command_racingdogsAppend('#racingdogs-games-table tbody', racingdogs_format_rows($variables));
        if (count($rows) >= 25) {
            $commands[] = ajax_command_replace('#div-get-more-bets', '<div id="div-get-more-bets">' . l(t('Daugiau statymų'), 'account/bets/racingdogs/more/' . ($page + 1) . '/nojs/', array('attributes' => array('id' => array('get-more-bets'), 'class' => array('use-ajax')))) . '</div>');
        } else {
            $commands[] = ajax_command_remove('#div-get-more-bets');
        }
        print ajax_render($commands);
        exit;
    } else {
        drupal_add_js('jQuery(document).ready(function () { Drupal.ajax.prototype.commands.racingdogsAppend =  function (ajax, response, status) {jQuery(response.selector).append(response.value);}; });', 'inline');
        $html .= $form;
        $html .= theme('table', $variables);
        if (count($rows) >= 25) {
            $html .= '<div id="div-get-more-bets">' . l(t('Daugiau statymų'), 'account/bets/racingdogs/more/' . ($page + 1) . '/nojs/', array('attributes' => array('id' => array('get-more-bets'), 'class' => array('use-ajax')))) . '</div>';
        }
        return $html;
    }
}

function racingdogs_drawGamesList_row($sessionData, $account, &$waiting_sessions) {
    $fields = array('startdt', 'enddt', 'payin', 'stake', 'win', 'payout');
    $fieldsCurrency = array('payin', 'stake', 'win', 'payout');
    $row = array();
    $row['id'] = 's_' . $sessionData->sessionid;
    if (!$sessionData->enddt) { //continue;
        foreach ($fields as $field) {
            $row['data'][$field] = array('class' => $field, 'data' => '...');
        }
        $row['data']['startdt']['data'] = date('Y-m-d H:i:s', strtotime(substr($sessionData->sessionid, 0, -3)));
        $row['data']['payin']['data'] = account_currency_convert_string($account->uid, $sessionData->amount, $sessionData->curr);
        $row['data']['more']['data'] = '<div class="ajax-progress ajax-progress-throbber"><div class="throbber">&nbsp;</div></div>';
        //$row['data']['more']['data'] = l(t('Užbaigti sesiją'), 'racingdogs/finish/'.$sessionData->sessionid.'/check/nojs/', array('attributes' => array('id' => array('update-rd-data'), 'class' => array('no-underline', 'use-ajax'))));
        $row['class'][] = 'waiting';
        $waiting_sessions[] = $sessionData->sessionid;
    } else {
        foreach ($fields as $field) {
            $row['data'][$field] = array('class' => $field, 'data' => (in_array($field, $fieldsCurrency) ? account_currency_convert_string($account->uid, $sessionData->$field, $sessionData->curr) : $sessionData->$field));
        }
        $row['data']['more'] = array('data' => l(t('Plačiau...'), 'account/bets/racingdogs/' . $sessionData->sessionid));
        if (floatval($sessionData->payout) > floatval($sessionData->payin)) {
            $row['class'][] = 'won';
        } elseif (floatval($sessionData->payout) < floatval($sessionData->payin)) {
            $row['class'][] = 'lost';
        } else {
            $row['class'][] = 'equal';
        }
    }
    return $row;
}

function ajax_command_racingdogsAppend($selector, $value) {
    return array(
      'command' => 'racingdogsAppend',
      'selector' => $selector,
      'value' => $value,
    );
}

function racingdogs_format_rows($variables, $tabletr = true) {
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
                    } else {
                        $attributes[$key] = $value;
                    }
                }
            } else {
                $cells = $row;
            }
            if (count($cells)) {
                // Add odd/even class
                if (empty($row['no_striping'])) {
                    $class = $flip[$class];
                    $attributes['class'][] = $class;
                }

                // Build row
                if ($tabletr) {
                    $output .= ' <tr' . drupal_attributes($attributes) . '>';
                }
                $i = 0;
                foreach ($cells as $cell) {
                    $cell = tablesort_cell($cell, $header, $ts, $i++);
                    $output .= _theme_table_cell($cell);
                }
                if ($tabletr) {
                    $output .= " </tr>\n";
                }
            }
        }
    }
    return $output;
}

function racingdogs_stats_details_page($account, $sessionid) {
    drupal_set_title(t('Mano erdvė'));

    $sessionData = racingdogs_getSessionInfo($sessionid, $account->data['tsuid']);
    if (!$sessionData OR !$sessionData->enddt) {
        drupal_set_message(t('Lošimas nerastas'), 'error');
        drupal_goto('account/bets/racingdogs');
        exit;
    }
    $html = '';
    $html .= l('<span class="glyphicon glyphicon-chevron-left"></span> '.t('Grįžti į ataskaitų sąrašą'), 'account/bets/racingdogs', array('html' => TRUE, 'attributes' => array('class' => array('btn btn-default pull-right'))));
    $html .= '<h3>' . t('Lošimo informacija') . '</h3>';
    $rows = array(
      array(array(
          'header' => TRUE,
          'data' => t('Lošimo Nr.'),
        ), array(
          'data' => $sessionData->sessionid,
        )),
      array(array(
          'header' => TRUE,
          'data' => t('Pradžia'),
        ), array(
          'data' => $sessionData->startdt,
        )),
      array(array(
          'header' => TRUE,
          'data' => t('Pabaiga'),
        ), array(
          'data' => $sessionData->enddt,
        )),
      array(array(
          'header' => TRUE,
          'data' => t('Įsinešta'),
        ), array(
          'data' => account_currency_convert_string($account->uid, $sessionData->payin, $sessionData->curr),
        )),
      array(array(
          'header' => TRUE,
          'data' => t('Statyta'),
        ), array(
          'data' => account_currency_convert_string($account->uid, $sessionData->stake, $sessionData->curr),
        )),
      array(array(
          'header' => TRUE,
          'data' => t('Laimėta'),
        ), array(
          'data' => account_currency_convert_string($account->uid, $sessionData->win, $sessionData->curr),
        )),
      array(array(
          'header' => TRUE,
          'data' => t('Išmokėta'),
        ), array(
          'data' => account_currency_convert_string($account->uid, $sessionData->payout, $sessionData->curr),
        )),
    );
    $html .= theme('table', array('rows' => $rows, 'attributes' => array('style' => array('width: 300px;'))));
    $html .= '<h3>' . t('Lažybų istorija') . '</h3>';

    if (0 and $cached = cache_get('racingdogs:session:' . $sessionid)) {
        $gamesData = $cached->data;
    } else {
        $gamesData = array();
        $sessionDetails = racingdogs_post_statusRequest($sessionid, true);
        if (isset($sessionDetails['@attributes'])) {
            if (isset($sessionDetails['rdTip'])) {
                if (isset($sessionDetails['rdTip']['@attributes'])) {
                    $sessionDetails['rdTip'] = array($sessionDetails['rdTip']);
                }
                foreach ($sessionDetails['rdTip'] as $tip) {
                    $rdTip = $tip['@attributes'];
                    if (strpos($rdTip['tip'], '/') === FALSE) {
                        $rdTip['title'] = t('Pirmas atbėgs šuo Nr. @dog1', array('@dog1' => $rdTip['tip']));
                    } else {
                        list($dog1, $dog2) = explode('/', $rdTip['tip']);
                        $rdTip['title'] = t('Pirmas atbėgs šuo Nr. @dog1, antras - Nr. @dog2', array('@dog1' => $dog1, '@dog2' => $dog2));
                    }
                    $gamesData[$tip['@attributes']['gameID']]['rows'][] = array(
                      'class' => array(($rdTip['win'] > 0 ? 'won' : 'lost')),
                      'data' => array(
                        $rdTip['tipID'],
                        $rdTip['title'],
                        $rdTip['tip'],
                        $rdTip['quote'],
                        account_currency_convert_string($account->uid, $rdTip['stake'], $sessionData->curr),
                       array('data' =>  account_currency_convert_string($account->uid, $rdTip['win'], $sessionData->curr) , 'class' => 'payout'),
                      ),
                    );
                    if (!isset($gamesData[$tip['@attributes']['gameID']]['date'])) {
                        $gamesData[$tip['@attributes']['gameID']]['date'] = $rdTip['tipDT'];
                    }
                    if (!isset($gamesData[$tip['@attributes']['gameID']]['total_stake'])) {
                        $gamesData[$tip['@attributes']['gameID']]['total_stake'] = 0;
                    }
                    $gamesData[$tip['@attributes']['gameID']]['total_stake'] += floatval($rdTip['stake']);
                    if (!isset($gamesData[$tip['@attributes']['gameID']]['total_win'])) {
                        $gamesData[$tip['@attributes']['gameID']]['total_win'] = 0;
                    }
                    $gamesData[$tip['@attributes']['gameID']]['total_win'] += floatval($rdTip['win']);
                }
            }
            cache_set('racingdogs:session:' . $sessionid, $gamesData, 'cache', CACHE_PERMANENT);
        }
    }
    if (empty($gamesData)) {
        $html .= '<p>' . t('Lažybų metu statymų nebuvo įvykdyta.') . '</p>';
    } else {
        $headers = array(t('Nr.'), t('Pavadinimas'), t('Variantas'), t('Koeficientas'), t('Statyta'), t('Laimėta'),);
        foreach ($gamesData as $key => $gameData) {
            $html .= '<h4>' . t('Lažybų kortelės Nr.: @number', array('@number' => $key)) . '</h4>';
            $html .= '<div class="rdgame-date">' . t('Data: @date', array('@date' => $gameData['date'])) . '</div>';
            $html .= theme('table', array('rows' => $gameData['rows'], 'header' => $headers, 'attributes' => array('class' => array('lost-won-colors'))));
            $html .= '<div class="rdgame-total-stake">' . t('Viso pastatyta: @value', array('@value' => account_currency_convert_string($account->uid, $gameData['total_stake'], $sessionData->curr))) . '</div>';
            $html .= '<div class="rdgame-total-win">' . t('Viso laimėta: @value', array('@value' => account_currency_convert_string($account->uid, $sessionData->curr))) . '</div>';
            $html .= '<hr/>';
        }
    }
    return $html;
}
