<?php

namespace oddsManager;

class TsApi {
	public static function isTablet() { return true; }
	public static function transliterationProcess($string, $unknown = '?', $source_langcode = NULL) {
  // ASCII is always valid NFC! If we're only ever given plain ASCII, we can
  // avoid the overhead of initializing the decomposition tables by skipping
  // out early.
  if (!preg_match('/[\x80-\xff]/', $string)) {
    return $string;
  }

  static $tail_bytes;
  

  if (!isset($tail_bytes)) {
    // Each UTF-8 head byte is followed by a certain number of tail bytes.
    $tail_bytes = array();
    for ($n = 0; $n < 256; $n++) {
      if ($n < 0xc0) {
        $remaining = 0;
      }
      elseif ($n < 0xe0) {
        $remaining = 1;
      }
      elseif ($n < 0xf0) {
        $remaining = 2;
      }
      elseif ($n < 0xf8) {
        $remaining = 3;
      }
      elseif ($n < 0xfc) {
        $remaining = 4;
      }
      elseif ($n < 0xfe) {
        $remaining = 5;
      }
      else {
        $remaining = 0;
      }
      $tail_bytes[chr($n)] = $remaining;
    }
  }

  // Chop the text into pure-ASCII and non-ASCII areas; large ASCII parts can
  // be handled much more quickly. Don't chop up Unicode areas for punctuation,
  // though, that wastes energy.
  preg_match_all('/[\x00-\x7f]+|[\x80-\xff][\x00-\x40\x5b-\x5f\x7b-\xff]*/', $string, $matches);

  $result = '';
  foreach ($matches[0] as $str) {
    if ($str[0] < "\x80") {
      // ASCII chunk: guaranteed to be valid UTF-8 and in normal form C, so
      // skip over it.
      $result .= $str;
      continue;
    }

    // We'll have to examine the chunk byte by byte to ensure that it consists
    // of valid UTF-8 sequences, and to see if any of them might not be
    // normalized.
    //
    // Since PHP is not the fastest language on earth, some of this code is a
    // little ugly with inner loop optimizations.

    $head = '';
    $chunk = strlen($str);
    // Counting down is faster. I'm *so* sorry.
    $len = $chunk + 1;

    for ($i = -1; --$len; ) {
      $c = $str[++$i];
      if ($remaining = $tail_bytes[$c]) {
        // UTF-8 head byte!
        $sequence = $head = $c;
        do {
          // Look for the defined number of tail bytes...
          if (--$len && ($c = $str[++$i]) >= "\x80" && $c < "\xc0") {
            // Legal tail bytes are nice.
            $sequence .= $c;
          }
          else {
            if ($len == 0) {
              // Premature end of string! Drop a replacement character into
              // output to represent the invalid UTF-8 sequence.
              $result .= $unknown;
              break 2;
            }
            else {
              // Illegal tail byte; abandon the sequence.
              $result .= $unknown;
              // Back up and reprocess this byte; it may itself be a legal
              // ASCII or UTF-8 sequence head.
              --$i;
              ++$len;
              continue 2;
            }
          }
        } while (--$remaining);

        $n = ord($head);
        if ($n <= 0xdf) {
          $ord = ($n - 192) * 64 + (ord($sequence[1]) - 128);
        }
        elseif ($n <= 0xef) {
          $ord = ($n - 224) * 4096 + (ord($sequence[1]) - 128) * 64 + (ord($sequence[2]) - 128);
        }
        elseif ($n <= 0xf7) {
          $ord = ($n - 240) * 262144 + (ord($sequence[1]) - 128) * 4096 + (ord($sequence[2]) - 128) * 64 + (ord($sequence[3]) - 128);
        }
        elseif ($n <= 0xfb) {
          $ord = ($n - 248) * 16777216 + (ord($sequence[1]) - 128) * 262144 + (ord($sequence[2]) - 128) * 4096 + (ord($sequence[3]) - 128) * 64 + (ord($sequence[4]) - 128);
        }
        elseif ($n <= 0xfd) {
          $ord = ($n - 252) * 1073741824 + (ord($sequence[1]) - 128) * 16777216 + (ord($sequence[2]) - 128) * 262144 + (ord($sequence[3]) - 128) * 4096 + (ord($sequence[4]) - 128) * 64 + (ord($sequence[5]) - 128);
        }
        $result .= self::transliterationReplace($ord, $unknown, $source_langcode);
        $head = '';
      }
      elseif ($c < "\x80") {
        // ASCII byte.
        $result .= $c;
        $head = '';
      }
      elseif ($c < "\xc0") {
        // Illegal tail bytes.
        if ($head == '') {
          $result .= $unknown;
        }
      }
      else {
        // Miscellaneous freaks.
        $result .= $unknown;
        $head = '';
      }
    }
  }
  return $result;
}
 public static function t($lang, $s) {return $s;}
}

class Cache {
	private static $PREFIX = 'ODDSM_';
	public function get($k) {
		$cache = cache_get(self::$PREFIX . $k);		
		if($cache && $cache->expire > REQUEST_TIME)	return $cache->data;
		return null;}
	public function set($k, $v, $expire) {
		cache_set(self::$PREFIX . $k, $v, 'cache', $expire);
	}
}

class OddsManager {
	
	private $db;
	private $cacheexpire;
	
    function __construct() {
        $this->cacheexpire = strtotime('+15 min');
    }

        protected static function getPageList($lang)
	{

		return array(			
			'superbet' => t('Superstatymas'),
			'septynetas' => t('Septynetas'),
			'last-minute' => t('Paskutinė minutė'),
			'today' => t('Lažybos šiandien'),
			'tomorrow' => t('Lažybos rytoj'),
            'weekend' => t('Savaitgalio lažybos'), 
		);
		
		$items['lt'] = array(
		'superbet' => 'Superstatymas',
		'septynetas' => 'Septynetas',
		'last-minute' => 'Paskutinė minutė',
		'today' => 'Lažybos šiandien',
		'tomorrow' => 'Lažybos rytoj',
	//	'all' => 'Lažybų pasiūla',
	//	'search' => 'Pasiūlos paieška',
		);
		$items['en-gb'] = array(
		'superbet' => 'SuperBet',
		'septynetas' => 'SevenBet',
		'last-minute' => 'Last minute',
		'today' => 'Betting today',
		'tomorrow' => 'Betting tomorrow',
	//	'all' => 'Betting offers',
	//	'search' => 'Betting search',
		);
		$items['ru'] = array(
		'superbet' => 'Суперставка',
		'septynetas' => 'Семерка',
		'last-minute' => 'Последняя минута',
		'today' => 'Пари сегодня',
		'tomorrow' => 'Пари завтра',
	//	'all' => 'Предложение пари',
	//	'search' => 'Поиск пари',
		);
		$items['lv'] = array(
		'superbet' => 'Superlikme',
		'septynetas' => 'Septiņu',
		'last-minute' => 'Pēdējā minūte',
		'today' => 'Derības šodien',
		'tomorrow' => 'Derības rīt',
	//	'all' => 'Derību piedāvājums',
	//	'search' => 'Поиск пари',
		);
		return $items[$lang];
	}
	
	public function getCacheExpire()
	{
		return $this->cacheexpire;
	}
    
    public function setCacheExpire($expire)
	{
        if($expire < $this->cacheexpire){
            $this->cacheexpire = $expire;
        }
	}


	protected function getConnection()
	{
		if($this->db) return $this->db;
		
		$opts = \Database::getConnection()->getConnectionOptions();
		$dbhost = $opts['host'];
		$dbport = $opts['port'];
		$dbname = $opts['database'];
		$dbuser = $opts['username'];
		$dbpass = $opts['password'];
		
		//echo "host=$dbhost port=$dbport dbname=$dbname user=$dbuser password=$dbpass";
		
		$this->db = pg_connect("host=$dbhost port=$dbport dbname=$dbname user=$dbuser password=$dbpass");
		
		return $this->db;
	}


	protected function additionalQueryParams($type, $query, $append = true, $searchkey = '')
	{
		$qry = '';
		
		if($type == 'popular' && strlen($searchkey))
			$qry .= ' and oe.eid IN (' . $searchkey .' )';
		
		if($type == 'favorite')
		{
			$teamid = (int) $searchkey;
			$qry .= ' and (oe.item1 = '.$teamid.' OR oe.item2 = '.$teamid.')';
		}
		
		if($type == 'superbet')
			$qry .= ' and oe.name <= 20 and oe.date > ' . time();
		
		if($type == 'search')
			$qry .= ' and oe.search_data LIKE \'%' . $this->escapeDbLike(TsApi::transliterationProcess ($searchkey)) .'%\' and oe.date > ' . time();
		
		if($type == 'septynetas')
			$qry .= ' and oe.aid = 16 and oe.date > ' . time();
		
		if($type == 'today')
			$qry .= ' and oe.name < 60000 and oe.aid != 16 and oe.date > ' . time() . ' and oe.date < ' . strtotime('tomorrow + 8 hours', time());
		
		if($type == 'tomorrow')
			$qry .= ' and oe.name < 60000 and oe.aid != 16 and oe.date > ' . strtotime('tomorrow - 1 sec', time()) . ' and oe.date < ' . strtotime('tomorrow + 32 hours', time());
		
		if($type == 'weekend')
			$qry .= ' and oe.name < 60000 and oe.aid != 16 and oe.date > ' . (in_array(date('N'), array(5, 6, 7)) ? time() : strtotime('next friday +17 hours', time())) 
				. ' and oe.date < ' . (date('N') == 7 ? strtotime('tomorrow', time()) : strtotime('next monday', time()));
		
		if($type == 'last-minute')
			$qry .= ' and oe.name < 60000 and oe.aid != 16 and oe.date > ' . time() . ' and oe.date < ' . strtotime('+3 hours', time());		
		
		if($type == 'all')
			$qry .= ' and oe.date > ' . time();
		
		
		if(!$append) $qry = substr ($qry, 4);
		
		return $query . $qry;
	}
			
	
	protected function escapeDbLike($str)
	{
		return strtolower(strtr($str, array('%'=>'\%', '_' => '\_')));
	}

	protected function getOffersCount($type)
	{
		$db = $this->getConnection();
		
		$tmp = explode('/' , $type);
		
		$type = $tmp[0];
		
		$args = array();
		$qry = "select count(*) from ts_offers_events oe
			where oe.language = 'lt' and oe.enabled = 1";
		$qry = $this->additionalQueryParams($type, $qry);		
		
		$cnt = count($tmp);
		if($cnt > 1) $qry .= ' and aid = ' . (int)$tmp[1];
		if($cnt > 2) $qry .= ' and cid = ' . (int)$tmp[2];
		if($cnt > 3) $qry .= ' and mid = ' . (int)$tmp[3];
		if($cnt > 4) $qry .= ' and peid = ' . (int)$tmp[4];
		
		$cnt = 0;
		if($result = pg_query($db, $qry))
			list($cnt) = pg_fetch_row($result);
		
		return $cnt;
	}

	protected function getCategories($lang, $catType, $type, $arg, $cat) {
        switch ($catType) {
            case 'area':
                $field = 'aid';
                break;
            case 'country':
                $field = 'cid';
                break;
            case 'module':
                $field = 'mid';
                break;
            case 'event':
                $field = 'eid';
                break;
        }


        $qry = 'select oe.' . $field . ', count(oe.eid) as count, min(oe.name) as weight, min(oe.date) as expire from ts_offers_events oe where language = $1 and enabled = 1';

        foreach ($cat as $k => $v)
            if ($v !== NULL)
                $qry .= ' and oe.' . $k . ' = ' . $v;
        $qry = $this->additionalQueryParams($type, $qry, true, $arg);
        $qry .= ' group by ' . $field;
        $qry .= ' order by weight';

        //global $db;

        $db = $this->getConnection();
        $result = pg_query_params($db, $qry, array($lang));

        $categories = array();
        if ($result) {
            while ($r = pg_fetch_row($result)) {
                $categories[$r[0]] = array('count' => $r[1], 'expire' => $r[3]);
                if ($field == 'aid'){
                    $categories[$r[0]]['icon'] = $r[0] . '.png';
                }
            }
        }
        if (count($categories)) {

            $result = pg_query_params($db, 'select distinct on (' . $field . ') ' . $field . ', category from ts_offers_events where ' . $field . ' IN (' . implode(',', array_keys($categories)) . ') and language = $1', array($lang));
            while ($r = pg_fetch_row($result)) {
                $categories[$r[0]]['title'] = $r[1];
            }
        }
        return $categories;
    }

    protected function buildCategoryLinks($categories, $path, $catType, &$expire)
	{
		$build = array();
		foreach($categories as $cid => $tmp){
			  if(!$cid)				  continue;
			  if((int)$tmp['expire'] < (int)$expire){
                  $expire = $tmp['expire'];                  
              }
			  $link = array(
			  'type' => 'category',
				'object' => array(
					'id' => $path . '/' . $cid,
					'name' => $this->getTitle($tmp['title'], $catType),
					'count' => $tmp['count'],					
				)				  
			);
			  if(TsApi::isTablet())
			  {
				  if($catType == 'country')
						$link['object']['favid'] = 'c' . $cid;
				  elseif($catType == 'module')
						$link['object']['favid'] = 'm' . $cid;
			  }
			  if(isset($tmp['icon'])) $link['object']['icon'] = $tmp['icon'];			  
			array_push($build , $link);
		  }
		  return $build;
	}


	protected function getCategoryLinks($lang, $path, $catType, $type, $arg, $cat)
	{
		$build = array();
		$cache = new Cache();
		$_key = 'mapi_' . implode(',', array(__CLASS__, __FUNCTION__,$lang, $path, $catType, $type, $arg, TsApi::isTablet() ? 'tablet' : 'phone', implode('_',$cat)));
		if($ob = $cache->get($_key))
		{			
			return $ob;
		}
		
		$categories = $this->getCategories($lang, $catType, $type, $arg, $cat);
		 
		$expire = $this->getCacheExpire();		
				
		  $build = $this->buildCategoryLinks($categories, $path, $catType, $expire);
		  $cache->set($_key, $build, $expire);
          
          $this->setCacheExpire($expire);
		  
		 return $build;
	}
			

	public function getTitle($cat, $type)
	{
		$r = explode(' » ', $cat);
		switch ($type)
		{
			case 'area': return $r[0];
			case 'country' : return $r[1];
			case 'module' : return $r[2];			
		}
		return $r[3];
	}
	
	public function wrapPatternOnSelection($gtid, $eventtitle, $item1, $item2, $fora1, $fora2, $selection)
	{
		$gt =$this->getGameTypes(array($gtid), $lang);
		$gt = array_shift($gt);
		$gtpids = array($gt['pattern']);
		$pattern = $this->getPatterns($gtpids, $lang);
		$pattern = array_shift($pattern);
		
		$o = array('item1name' => $item1, 'item2name' => $item2, 'totalforaresult1' => $fora1, 'totalforaresult2' => $fora2, 'title' => $eventtitle);
        $pdat = $this->eventPatternData($o, $lang);
				
		for($i=0;$i<6;$i++)
			if($gt['cn'.$i] == $selection)
				return strtr($patterns[$i], $pdat);
		return $selection;
	}
	
	
	

	protected function getGameTypes($gtids, $language)
	{
		
		$db = $this->getConnection();
		if(!is_array($gtids) || !count($gtids)) return array();
		$gtypes = array();
		$result = pg_query_params($db, "select gtid, cn1, cn2, cn3, cn4, cn5, cn6, title, patternid, g_ext, collapsed from ts_offers_gametypes WHERE language = $1 AND gtid IN (".implode(',', $gtids).") ORDER BY weight", array($language));
		while($r = pg_fetch_row($result))
		{
			$gtypes[$r[0]] = array($r[1],$r[2],$r[3],$r[4],$r[5],$r[6], 'title' => $r[7], 'pattern' => $r[8], 'g_ext' => $r[9], 'collapsed' => $r[10] == 't');
		}
		
		
		
		return $gtypes;
	}
    
    protected function getGameTypesGroups($pgtids, $language)
	{        
		$gtids = array_keys($pgtids);
		$db = $this->getConnection();
		if(!is_array($gtids) || !count($gtids)) return array();
		$gtypes = array();
		$result = pg_query_params($db, "select id, title from ts_offers_gametypes_groups WHERE language = $1 AND id IN (".implode(',', $gtids).") ORDER BY weight", array($language));
		while($r = pg_fetch_row($result))
		{
			$gtypes[$r[0]] = array('title' => $r[1], 'children' => $pgtids[$r[0]]['children'], 'total' => $pgtids[$r[0]]['total']);
		}		
		
		return $gtypes;
	}
	
	protected function getPatterns($gtids, $language)
	{
		$db = $this->getConnection();
		if(!is_array($gtids) || !count($gtids)) return array();
		$gtypes = array();
		$result = pg_query_params($db, "select gtpid, cn1, cn2, cn3, cn4, cn5, cn6 from ts_offers_gametypes_patterns WHERE language = $1 AND gtpid IN (".implode(',', $gtids).")", array($language));
		while($r = pg_fetch_row($result))
		{
			$gtypes[$r[0]] = array($r[1],$r[2],$r[3],$r[4],$r[5],$r[6]);
		}
		return $gtypes;
	}

    private function getModules($lang, $path, $type, $arg, $aid = null, $cid = null, $mid = null, $eid = null) 
	{
       			
		$cache = new Cache();
		$_key = 'mapi_' . implode(',', array(__CLASS__, __FUNCTION__,$lang, $path, $type, $arg, $aid, $cid, $mid, $eid, TsApi::isTablet() ? 'tablet' : 'phone'));
		if($ob = $cache->get($_key))
		{			
			return $ob;
		}	
		
				
		  $cat = array('aid' => $aid, 'cid' => $cid, 'mid' => $mid, 'eid' => $eid);
		  $qry = 'select oe.eid, oe.aid,  oe.cid, oe.mid, oe.peid, oe.gtid, oe.date, oe.startdate, oe.title, 
                oe.c1, oe.c2, oe.c3, oe.c4, oe.c5, oe.c6, oe.category , oe.totalforaresult1, oe.totalforaresult2, 
                oe.item1name, oe.item2name, oe.item1, oe.item2, oe.e_ext, count(ce.eid) as count, min(oe.name) as weight, 
                oe.item1country_id, oe.item2country_id, oe.combtosamearea, oe.combtosamemodule 
			  from ts_offers_events oe 
			  left join ts_offers_events ce on ( ' . ($type == 'superbet' ?  'ce.peid = oe.peid' : 'ce.peid = oe.eid' ) . ' and ce.language = oe.language and ce.enabled = 1 )
			where oe.language = $1 and oe.enabled = 1';
		  
		  if($type != 'superbet') $qry .= ' and oe.peid is null';
		  
		  $qry .= ' and oe.date > $2';
		  
		  foreach ($cat as $k => $v)
			  if($v !== NULL)
				$qry .= ' and oe.'.$k.' = '.$v;
		  $qry = $this->additionalQueryParams($type, $qry, true, $arg);
		  $qry .= ' group by oe.eid, oe.aid,  oe.cid, oe.mid, oe.peid, oe.gtid, oe.date, oe.startdate, oe.title, '
                    . 'oe.c1, oe.c2, oe.c3, oe.c4, oe.c5, oe.c6, oe.e_ext, oe.category, oe.totalforaresult1, oe.totalforaresult2, oe.item1name, '
                    . 'oe.item2name, oe.item1, oe.item2, oe.item1country_id, oe.item2country_id, oe.combtosamearea, oe.combtosamemodule';
		  $qry .= ' order by weight';
		  $db = $this->getConnection();
		  
		  $offers = array();
		  $result = pg_query_params($db, $qry, array($lang, time() + 60));
		  $categories = array();
		  $gtids = array();
		  while ($r = pg_fetch_assoc($result))
		  {
			  $offers[$r['eid']] = $r;
			  $gtids[$r['gtid']] = $r['gtid'];
		  }
		  $gtypes = $this->getGameTypes($gtids, $lang);
		  $gtpids = array();
		  foreach ($gtypes as $gt)
			  $gtpids[$gt['pattern']] = $gt['pattern'];
		  $patterns = $this->getPatterns($gtpids, $lang);
		  $build = array();
		  
		  foreach ($offers as $o)
		  {
			  $link = array(
                'type' => 'betting',
				'object' => array(
					'id' => $o['eid'],
					'aid' => $o['aid'],
					'cid' => $o['cid'],
					'mid' => $o['mid'],
					'pid' => $o['peid'],
					'title' => $o['title'],
                    'item1name' => $o['item1name'],
                    'item2name' => $o['item2name'],
                    'item1country_id' => $o['item1country_id'],
                    'item2country_id' => $o['item2country_id'],
					'name' => $o['weight'],
					'category' => $o['category'],
					'gametype' => $gtypes[$o['gtid']]['title'],
					'g_ext' => $gtypes[$o['gtid']]['g_ext'],
					'e_ext' => $o['e_ext'],
					'collapsed' => $gtypes[$o['gtid']]['collapsed'],
					'gtid' => $o['gtid'],
					'advanced_bet' => $o['count'],
					'date' => date('Y-m-d H:i', $o['startdate']),
					'bets' => array(),
                    'combtosamearea' => $o['combtosamearea'] ? true : false,
                    'combtosamemodule' => $o['combtosamemodule'] ? true : false,
				)
			);
			  
			   if(TsApi::isTablet())
			  {
				  $link['object']['favid1'] = 't' . $o['item1'];
				  if($o['item2']) $link['object']['favid2'] = 't' . $o['item2'];
			  }
			  
			  $pdat = $this->eventPatternData($o, $lang);
			  
			  $p1case = true; // is it case with repeating <P1> bets ?
			  $p12case = true;
			  $advanceit = false;
				for($i=0;$i<6;$i++)
					if($o['c'.($i+1)])
					{
						//if($gtypes[$o['gtid']][$i] !== '10' && $gtypes[$o['gtid']][$i] !== '02') 
							$link['object']['bets'][] = array('id'=>($i+1), 'tsid' => $gtypes[$o['gtid']][$i], 'pattern'=>$patterns[$gtypes[$o['gtid']]['pattern']][$i], 'title'=>strtr($patterns[$gtypes[$o['gtid']]['pattern']][$i], $pdat), 'odds'=>$o['c'.($i+1)]);
							$tp = explode('|', $patterns[$gtypes[$o['gtid']]['pattern']][$i]);
							$tp = array_shift($tp);							
							if( strpos($tp, '<P1>') !== 0 || strpos($tp, '<P2>') !== FALSE) $p1case = false;
							if( strpos($tp, "<P1> - <P2>") !== 0) $p12case = false;
						//else
						//	$advanceit = true;
                    }
					
			if($p1case)
			{
				$link['object']['bets_prefix'] = $pdat['<P1>'];
			}
			else
			if($p12case)
			{
				$link['object']['bets_prefix'] = $pdat['<P1>'] . ' - ' . $pdat['<P2>'];
			}
			//if( $advanceit ) // has "hidden" bets
			//	$link['object']['advanced_bet']++;
			
			array_push($build , $link);
		  }		  
		  
		  $cache->set($_key, $build, strtotime('+5 min'));
		  
		  return $build;
	}
	
	public function getUsedGametypes($lang, $args)
	{
		$type = null;
		$aid = null;
		$cid = null;
		$mid = null;
		$eid = null;
		
		$vars = array('type','aid','cid','mid','eid');
		$i = 0;
		foreach ($args as $v)
		{
			${$vars[$i++]} = $v;			
		}
		

		$cache = new Cache();
		$_key = 'mapi_' . implode(',', array(__CLASS__, __FUNCTION__,$lang, $type, $aid, $cid, $mid, $eid));
		if($ob = $cache->get($_key))
		{			
			return $ob;
		}	
		
			$qparams = array($lang);
			
		 // $cat = array('aid' => $aid, 'cid' => $cid, 'mid' => $mid, 'eid' => $eid);
		  $qry = 'select oe.gtid, MIN(oe.date) as expire, count(*) as total
			  from ts_offers_events oe 			  
			where oe.language = $1 and oe.enabled = 1';
		  
		  $pcnt = 2;
		  if($eid)
		  {
			  $qry .= ' and (oe.peid = $'.$pcnt.' or oe.eid = $'.$pcnt.')';
			  $qparams[] = $eid;
			  $pcnt++;
		  }
		  
		  if($aid) 
		  {
			  $qry .= ' and oe.aid = $'.$pcnt;
			  $qparams[] = $aid;
			  $pcnt++;
		  }
		  
		  if($cid) 
		  {
			  $qry .= ' and oe.cid = $'.$pcnt;
			  $qparams[] = $cid;
			  $pcnt++;
		  }
		  
		  if($mid) 
		  {
			  $qry .= ' and oe.mid = $'.$pcnt;
			  $qparams[] = $mid;
			  $pcnt++;
		  }
		  
		  $qry .= ' and oe.date > $'.$pcnt;
			  $qparams[] = time()+60;
			  $pcnt++;
		  
          //excluding already grouped GT's to groups
		  $qry .= ' AND oe.gtid not IN(SELECT gtid FROM ts_offers_gt_rel)';
          
		  $qry = $this->additionalQueryParams($type, $qry, true);
		  
		  $qry .= ' group by oe.gtid';
		  
		  $db = $this->getConnection();
		  
		  $result = pg_query_params($db, $qry, $qparams);
		  $gtids = array();
		  $gtcounts = array();
		  $expire = strtotime('+15 min');	
		  while($r = pg_fetch_object($result))
		  {
			  $gtids[] = $r->gtid;
			  $gtcounts[$r->gtid] = $r->total;
			  $expire = min(array($expire, $r->expire));
		  }
		  
		  $gts = array();		  
		  foreach ($this->getGameTypes($gtids, $lang) as $id => $g)
		  {
			  $g['total'] = $gtcounts[$id];
			  $gts[$id] = $g;
		  };
		  
		  $cache->set($_key, $gts, $expire);
		
		return $gts;
	}
    
    public function getUsedGametypesGroups($lang, $args){
        
		$type = null;
		$aid = null;
		$cid = null;
		$mid = null;
		$eid = null;
		
		$vars = array('type','aid','cid','mid','eid');
		$i = 0;
		foreach ($args as $v)
		{
			${$vars[$i++]} = $v;			
		}
		
		$cache = new Cache();
		$_key = 'mapi_' . implode(',', array(__CLASS__, __FUNCTION__,$lang, $type, $aid, $cid, $mid, $eid));
		if($ob = $cache->get($_key))
		{			
			return $ob;
		}	
		
			$qparams = array($lang);
			
		 // $cat = array('aid' => $aid, 'cid' => $cid, 'mid' => $mid, 'eid' => $eid);
		  $qry = 'SELECT oe.gtid,min(oe.date) as expire, count(*) as total, ogr.id as pgtid
			  FROM ts_offers_events oe 	
              LEFT JOIN ts_offers_gt_rel ogr ON ogr.gtid = oe.gtid		  
                WHERE oe.language = $1 and oe.enabled = 1';
		  
		  $pcnt = 2;
		  if($eid)
		  {
			  $qry .= ' and (oe.peid = $'.$pcnt.' or oe.eid = $'.$pcnt.')';
			  $qparams[] = $eid;
			  $pcnt++;
		  }
		  
		  if($aid) 
		  {
			  $qry .= ' and oe.aid = $'.$pcnt;
			  $qparams[] = $aid;
			  $pcnt++;
		  }
		  
		  if($cid) 
		  {
			  $qry .= ' and oe.cid = $'.$pcnt;
			  $qparams[] = $cid;
			  $pcnt++;
		  }
		  
		  if($mid) 
		  {
			  $qry .= ' and oe.mid = $'.$pcnt;
			  $qparams[] = $mid;
			  $pcnt++;
		  }
		  
		  $qry .= ' and oe.date > $'.$pcnt;
			  $qparams[] = time()+60;
			  $pcnt++;
          $qry .= ' AND oe.gtid IN(SELECT gtid FROM ts_offers_gt_rel)';
		  $qry = $this->additionalQueryParams($type, $qry, true);
		  
		  $qry .= ' group by oe.gtid, ogr.id';
		  
		  $db = $this->getConnection();
		  
		  $result = pg_query_params($db, $qry, $qparams);
		  $pgtids = array();
		  $expire = strtotime('+15 min');	
		  while($r = pg_fetch_object($result))
		  {
			  $pgtids[$r->pgtid]['children'][] = $r->gtid;
              $pgtids[$r->pgtid]['total'] = $r->total + (isset($pgtids[$r->pgtid]['total']) ? $pgtids[$r->pgtid]['total'] : 0);
			  $expire = min(array($expire, $r->expire));
		  }
		  $gts = $this->getGameTypesGroups($pgtids, $lang);
		  
		  $cache->set($_key, $gts, $expire);
		
		return $gts;
    }

    /**
     * 
     * @param type $lang
     * @param type $type
     * @param type $arg
     * @param type $aid
     * @param type $cid
     * @param type $mid
     * @param type $eid
     * @return array
     */
    private function getEvents($lang, $type, $arg, $aid = null, $cid = null, $mid = null, $eid = null) 
	{
		
		$cache = new Cache();
		$_key = 'mapi_' . implode(',', array(__CLASS__, __FUNCTION__,$lang, $type, is_array($arg) ? implode($arg) : $arg, $aid, $cid, $mid, $eid, TsApi::isTablet() ? 'tablet' : 'phone'));
		if($ob = $cache->get($_key))
		{			
			return $ob;
		}	
		
			$qparams = array($lang, time() + 60);
			
		  $qry = 'select oe.eid, oe.gtid, oe.date, oe.startdate, oe.title, oe.c1, oe.c2, oe.c3, oe.c4, oe.c5, oe.c6, oe.category, oe.name,
			  oe.totalforaresult1, oe.totalforaresult2, oe.item1name, oe.item2name, oe.item1, oe.item2, oe.peid, oe.e_ext, oe.event_count,
              oe.item1country_id, oe.item2country_id, oe.aid, oe.combtosamearea, oe.combtosamemodule
			  from ts_offers_events oe 			  
			where oe.language = $1 and oe.enabled = 1 and oe.date > $2';
		  
		  if($type == 'list' )
		  {
			  
			 $qry .= ' and eid in (' . implode(',', $arg). ')';			  
		  }
		  
		  if($eid)
		  {
			  $qry .= ' and (oe.peid = $3 or oe.eid = $3)';
			  $qparams[] = $eid;
		  }
		  
		  
		  
		  $qry = $this->additionalQueryParams($type, $qry, true, $arg);
		  
		  $qry .= ' order by oe.name';
		  
		  $db = $this->getConnection();
		  
		  $result = pg_query_params($db, $qry, $qparams);
		  $categories = array();
		  $gtids = array();
		  $offers = array();
		  while ($r = pg_fetch_assoc($result))
		  {
			  $offers[$r['eid']] = $r;
			  $gtids[$r['gtid']] = $r['gtid'];
		  }
		  $gtypes =$this->getGameTypes($gtids, $lang);
		  $gtpids = array();
		  foreach ($gtypes as $gt)
			  $gtpids[$gt['pattern']] = $gt['pattern'];
		  $patterns = $this->getPatterns($gtpids, $lang);
		  $build = array();
		  
		  $expire = strtotime('+15 min');
		  
		  foreach ($offers as $o)
		  {
			  if($o['date'] < $expire) $expire = $o['date'];
			  $link = array(
                'type' => 'betting',
                'object' => array(
                    'id' => $o['eid'],
                    'pid' => $o['peid'],
                    'title' => $o['title'],
                    'item1name' => $o['item1name'],
                    'item2name' => $o['item2name'],
                    'item1country_id' => $o['item1country_id'],
                    'item2country_id' => $o['item2country_id'],
                    'name' => $o['name'],
                    'category' => $o['category'],
                    'gametype' => $gtypes[$o['gtid']]['title'],					
                    'g_ext' => $gtypes[$o['gtid']]['g_ext'],
                    'e_ext' => $o['e_ext'],
                    'collapsed' => $gtypes[$o['gtid']]['collapsed'],
                    'gtid' => $o['gtid'],
                    'aid' => $o['aid'],
                    'advanced_bet' => 0,
                    'date' => date('Y-m-d H:i', $o['startdate']),
                    'bets' => array(),
                    'combtosamearea' => $o['combtosamearea'] ? true : false,
                    'combtosamemodule' => $o['combtosamemodule'] ? true : false,
                )
			);
			  
			  if(TsApi::isTablet())
			  {
				  $link['object']['favid1'] = 't' . $o['item1'];
				  if($o['item2']) $link['object']['favid2'] = 't' . $o['item2'];
			  }
			  
			  $p1case = true;
			  $p12case = true;
			  $pdat = $this->eventPatternData($o, $lang);
				for($i=0;$i<6;$i++)
				{
					if($o['c'.($i+1)])
					{
						$link['object']['bets'][] = array('id'=>($i+1), 'tsid' => $gtypes[$o['gtid']][$i], 'pattern'=>$patterns[$gtypes[$o['gtid']]['pattern']][$i], 'title'=>strtr($patterns[$gtypes[$o['gtid']]['pattern']][$i], $pdat), 'odds'=>$o['c'.($i+1)]);
						$tp = explode('|', $patterns[$gtypes[$o['gtid']]['pattern']][$i]);
						$tp = array_shift($tp);
					
						if( strpos($tp, '<P1>') !== 0 || strpos($tp, '<P2>') !== FALSE) $p1case = false;
						if( strpos($tp, "<P1> - <P2>") !== 0) $p12case = false;
					}
				}
			
				if($p1case)
				{
					$link['object']['bets_prefix'] = $pdat['<P1>'];
				} else
				if($p12case)
				{
					$link['object']['bets_prefix'] = $pdat['<P1>'] . ' - ' . $pdat['<P2>'];
				}
			array_push($build , $link);
		  }		  
		  
		  $cache->set($_key, $build, $expire);
          $this->setCacheExpire($expire);
		  return $build;
	}

protected function eventPatternData($event, $lang){

  $pattern = array(
    '<P1>' => $event['item1name'],
    '<P2>' => $event['item2name'],
    '<F1>' => '<span class="fora">'.round($event['totalforaresult1'], 2).'</span>',
    '<F2>' => '<span class="fora">'.round($event['totalforaresult2'], 2).'</span>',
    '<E1>' => $event['title'],
  //     '<E1>' => '<font color="red">[Exception E1]</font>'.' <b>'.$event->title.'</b> Fora1-'.$event->totalforaresult1.' Fora2-'.$event->totalforaresult2,
  );

  /* Exception E1 */
  if (($event['totalforaresult1'] == 1) and ($event['totalforaresult2'] == 1)){ $pattern['<E1>'] = t("<P1> - <P2>: Ir pirmą kėlinį, ir rungtynes laimės <span class=\"fora\"><P1></span>"); }
  if (($event['totalforaresult1'] == 1) and ($event['totalforaresult2'] == 0)){ $pattern['<E1>'] = t("<P1> - <P2>: Pirmą kėlinį laimės <span class=\"fora\"><P1></span>, o rungtynės baigsis lygiosiomis"); }
  if (($event['totalforaresult1'] == 1) and ($event['totalforaresult2'] == 2)){ $pattern['<E1>'] = t("<P1> - <P2>: Pirmą kėlinį laimės <span class=\"fora\"><P1></span>, o rungtynės laimės <span class=\"fora\"><P2></span>"); }
  if (($event['totalforaresult1'] == 0) and ($event['totalforaresult2'] == 1)){ $pattern['<E1>'] = t("<P1> - <P2>: Pirmas kėlinys baigsis lygiosiomis, o rungtynes laimės <span class=\"fora\"><P1></span>"); }
  if (($event['totalforaresult1'] == 0) and ($event['totalforaresult2'] == 0)){ $pattern['<E1>'] = t("<P1> - <P2>: Ir pirmas kėlinys, ir visos rungtynės baigsis lygiosiomis"); }
  if (($event['totalforaresult1'] == 0) and ($event['totalforaresult2'] == 2)){ $pattern['<E1>'] = t("<P1> - <P2>: Pirmas kėlinys baigsis lygiosiomis, o rungtynes laimės <span class=\"fora\"><P2></span>"); }
  if (($event['totalforaresult1'] == 2) and ($event['totalforaresult2'] == 1)){ $pattern['<E1>'] = t("<P1> - <P2>: Pirmą kėlinį laimės <span class=\"fora\"><P2></span>, o rungtynes laimės <span class=\"fora\"><P1></span>"); }
  if (($event['totalforaresult1'] == 2) and ($event['totalforaresult2'] == 0)){ $pattern['<E1>'] = t("<P1> - <P2>: Pirmą kėlinį laimės <span class=\"fora\"><P2></span>, o rungtynės baigsis lygiosiomis"); }
  if (($event['totalforaresult1'] == 2) and ($event['totalforaresult2'] == 2)){ $pattern['<E1>'] = t("<P1> - <P2>: Ir pirmą kėlinį, ir rungtynes laimės <span class=\"fora\"><P2></span>"); }
  if ($event['totalforaresult1'] == 999 && $event['totalforaresult2'] == 0) 
	{
	  unset($pattern['<F1>']);
	  unset($pattern['<F2>']);
	  $pattern['<F1>:<F2>'] = _('kitas');
	}
    
  $shortversion_exploded = explode(':', $pattern['<E1>']);
  $shortversion = array_pop($shortversion_exploded);
  
  $pattern['<E1>'] = strtr($pattern['<E1>'] . '|' . $shortversion, $pattern);

  return $pattern;

}

protected function getFavoriteCategoryLinks($lang, $uid, $catType, $path, $aid = null, $cid = null, $mid = null)
{
	$cat = array();
	if($aid !== NULL) $cat['aid'] = $aid;
	if($cid !== NULL) $cat['cid'] = $cid;
	if($mid !== NULL) $cat['mid'] = $mid;
	return $this->getCategoryLinks($lang, $path, $catType, 'all', '', $cat);
}

protected function getFavoriteOffers($lang, $uid, $type, $refid, $cat)
{
	require_once 'favorites.php';
	
	$refid = (int) $refid;
	if($type == 'c') 
	{
		switch (count($cat))
		{
			case 0 : return $this->getFavoriteCategoryLinks($lang, $uid, 'area', 'favorite/'.$type.$refid, null, $refid);
			case 1 : return $this->getFavoriteCategoryLinks($lang, $uid, 'module','favorite/'.$type.$refid.'/'.$cat[0],  $cat[0], $refid);
			case 2 : return $this->getModules($lang, 'favorite/'.$type.$refid.'/'.$cat[0].'/'.$cat[1], 'all', '', $cat[0], $refid, $cat[1]);
			case 3 : return $this->getEvents($lang, 'all', '', $cat[0], $refid, $cat[1], $cat[2]);
		}
	}
	
	if($type == 'm') 
	{
		switch (count($cat))
		{	
			case 0 : return $this->getModules($lang, 'favorite/'.$type.$refid, 'all', '', null, null, $refid);
			case 1 : return $this->getEvents($lang, 'all', '', null, null, $refid, $cat[0]);
		}
	}
	
	if($type == 't')
	{
		$cnt = count($cat);
		switch (count($cat))
		{			
			case 0 : return $this->getCategoryLinks($lang,'favorite/'.$type.$refid, 'module', 'favorite', $refid, array());
			case 1 : return $this->getModules($lang, 'favorite/'.$type.$refid.'/'.$cat[0], 'favorite', $refid, null, null, $cat[0]);
			case 2 : return $this->getEvents($lang, 'favorite', $refid, null, null, $cat[0], $cat[1]);
		}
	}
}

protected function getOffers($lang, $type, $style, $arg , $aid = null, $cid = null, $mid = null, $eid = null)
	{
	
		if($type == 'list')
		{
			if(!count($arg)) return array();
			return $this->getEvents($lang, $type, explode(',', $arg));
		}
		if($aid === null){
			$catType = 'area';
			$catTypePrevious = null;
			$path = $type .($arg ? '/'.$arg : '') ;
		  }elseif($cid === null){
			$catType = 'country';
			$catTypePrevious = 'area';
			$path = $type.($arg ? '/'.$arg : '').'/'.$aid;
		  }elseif($mid === null){
			$catType = 'module';
			$catTypePrevious = 'country';
			$path = $type.($arg ? '/'.$arg : '').'/'.$aid .'/'. $cid ;
		  }elseif($eid === null){
			  $path = $type.($arg ? '/'.$arg : '').'/'.$aid .'/'. $cid .'/'. $mid ;
			return $this->getModules($lang, $path, $type, $arg, $aid, $cid, $mid);
		  }elseif($eid !== null){
			return $this->getEvents($lang, $type, $arg, $aid, $cid, $mid, $eid);
		  }
		  $cat = array('aid' => $aid, 'cid' => $cid, 'mid' => $mid, 'eid' => $eid);
		  
		  $build = array();

		/*if($catType == 'country' AND in_array($type, array('last-minute', 'today', 'tomorrow'))){
		   $build = $this->getCategoryLinks($lang, $path, $catType, $type, $arg, $cat);		  
		  
		}else{*/
		  if($catType == 'country'){
			$catWorld = array('aid' => $aid, 'cid' => 0, 'mid' => null, 'eid' => null);
			$build = $this->getCategoryLinks($lang,$path.'/0', 'module', $type, $arg, $catWorld);
		  }

		  $build = array_merge($build, $this->getCategoryLinks($lang, $path, $catType, $type, $arg, $cat));
 
		//}

		return $build;
	}


	public function getPopularOffers($lang, $peid = 0, $limit = 5)
	{
		$cache = new Cache();
		$_key = 'mapi_' . implode(',', array(__CLASS__, __FUNCTION__,$lang, $peid));
		if($ob = $cache->get($_key))
		{			
			return $ob;
		}	
		
		$expire = strtotime('+15 min');
		
		if($peid)
		{
			
			$events = $this->getEvents($lang, 'popular', null, null, null, null, $peid);
			foreach ($events as $e)
			{
				$tm = strtotime($e['object']['date']);	
				if($tm < $expire) $expire = $tm;
			}
			
			$cache->set($_key, $events, $expire);
            $this->setCacheExpire($expire);
			return $events;
		}
		
		$response = topsport_getTopBetEvents($limit);
		//$response = TsApi::myrequest('https://topsport-proxy.data.lt/webservices/getTopBetEventsXML.jsp?limit=5');
		//$xlang = TsApi::topsportXmlLangCodes($lang);
		$xlang = strtoupper(substr($lang, 0, 2));
		
		//$response = json_decode(json_encode((array) simplexml_load_string($response)), 1);
		$rez = array("type"=>"plainbets","objects"=>array());
		$eids = array();
		foreach ($response['Areas']['Area'] as $ar)
		{
			$a = array("id"=>$ar['AreaId'], 'title'=>$ar['AreaName'][$xlang], 'events' => array());
			$evs = isset($ar['Events']['Event']['EventId']) ? array($ar['Events']['Event']) : $ar['Events']['Event'];
			foreach($evs as $e)
			{
				array_unshift($a['events'] , array('id'=>$e['EventId'], 'title'=>$e['EventString'][$xlang]));
				array_push($eids, $e['EventId']);
			}
			array_push($rez['objects'], $a);
		}
		
		$events = array();
		foreach ($this->getModules($lang, 'popular', 'popular', implode(',', $eids)) as $r)
		{
			$events[$r['object']['id']] = $r;
		}		
		foreach ($rez['objects'] as $a => $ofs)
		{
			$ob = array();
			foreach ($ofs['events'] as $oid => $o)
				if(array_key_exists ($o['id'], $events))
				{
					$ob[] = $events[$o['id']];
					//$rez['objects'][$a]['events'][$oid] = $events[$o['id']];
					$tm = strtotime($events[$o['id']]['object']['date']);
					if($tm < $expire) $expire = $tm;
				}
				/*else
				{
					unset($rez['objects'][$a]['events'][$oid]);
				}*/
			$rez['objects'][$a]['events'] = $ob;
		}
		
		$cache->set($_key, $rez, $expire);
		$this->setCacheExpire($expire);
		return $rez;
	}

	private function getMain($lang)
	{
		
		$pages = $this->getPageList($lang);
		$links = array();		
		foreach($pages as $key => $title)
		{
			$count = $this->getOffersCount($key);			
			if($count == 0){ continue; }
			$icon = $key.'.png';			
			$link = array(
			  'type' => 'category',
				'object' => array(
					'id' => $key,
					'name' => $title,
					'count' => $count,
					'icon' => $icon,
				)
			);
			$links[] = $link;			
		  }
		
		return $links;
	}
	
	public function getById($lang, $classicMode, $cat, $uid = false)
	{
		$arg = null;
		
		if($cat == null) return $this->getMain($lang);
		
		
		if($cat[0] == 'search') {$arg = $cat[1];array_splice($cat, 1, 1);}
		if($cat[0] == 'list') {$arg = $cat[1];array_splice($cat, 1, 1);}
		if($cat[0] == 'favorite') {			
			$type = substr($cat[1],0,1);
			$refid = substr($cat[1],1);
			array_splice($cat, 0, 2);			
			return $this->getFavoriteOffers($lang, $uid, $type, $refid, $cat);
			
		}
		if($cat[0] == 'popular') {
			if(count($cat)==2)
				return $this->getPopularOffers($lang, $cat[1]);
			return $this->getPopularOffers($lang);
		}
		$cnt = 0;
		foreach ($cat as $c)
			if($c !== null)
				$cnt++;
			else break;
		
		switch ($cnt)
		{
			case 1:
				return $this->getOffers($lang, $cat[0], $classicMode, $arg);
				break;
			case 2:
				return $this->getOffers($lang, $cat[0],  $classicMode, $arg, $cat[1]);
				break;
			case 3:
				return $this->getOffers($lang, $cat[0],  $classicMode, $arg, $cat[1], $cat[2]);
				break;
			case 4:
				return $this->getOffers($lang, $cat[0],  $classicMode, $arg, $cat[1], $cat[2], $cat[3]);
				break;
			case 5:
				return $this->getOffers($lang, 'all' /*$cat[0]*/,  $classicMode, $arg, $cat[1], $cat[2], $cat[3], $cat[4]);
				break;
			default:
				break;
		}
	}

	public function getEventByEventName($event_name, $lang){
        $data = false;
        $db = $this->getConnection();
        $qry = 'Select * FROM ts_offers_events WHERE name = $1 and language = $2 LIMIT 1';
        $qparams[0] = $event_name;
        $qparams[1] = $lang;
        $result = pg_query_params($db, $qry, $qparams);       
        $r = pg_fetch_assoc($result);
        if(isset($r['eid'])){
            $data = $this->getEvents($lang, 'list', array($r['eid']));
            if($data){
                $url = array($r['aid'], $r['cid'], $r['mid'], $r['peid'] ? $r['peid'] : $r['eid']);
                $data[0]['object']['url'] = implode('/', $url);
            }
        }        
        return $data;
    }
	
}