<?php
$oddTotal = count($offerData['odds']);
$rowspan = ($oddTotal > 3) ? 2 : 1;$rowspan = 1;
$offerData['category'] = htmlspecialchars($offerData['category']);
?>
<?php /* GAME MATCH STYLE */  if($eid) :?>
    <tr id="e_<?=$offerData['id'];?>" class="<?=(count($offerData['odds']) > 1 ? "offers_line" : "");?>"> 
        <?php
        $html = '';
        $k = 0;
        
		$colspan = ($oddTotal > 3) ? 2 : (6 / $oddTotal);
		
		if(isset($offerData['bets_prefix']))
		{	
			$cor_pattern = explode('|', $offerData['odds'][0]['title']);
            if(!$filtered && count($cor_pattern) > 1) {  $cor_pattern = $cor_pattern[1]; } else $cor_pattern = $cor_pattern[0];
			if(strpos($cor_pattern, $offerData['bets_prefix']) === 0 )
			{
				$html .= '<td class="betsPrefix">'.$offerData['bets_prefix'].'</td>';
				$colspan = ($oddTotal > 3) ? 3 : (6 / $oddTotal + 1);
				$betPrefixLen = strlen($offerData['bets_prefix']);
			}
			else
			{
				unset($offerData['bets_prefix']);
			}
		}
		
		
		
        foreach($offerData['odds'] as $odds){

            $cor_pattern = explode('|', $odds['title']);
            if(!$filtered && count($cor_pattern) > 1)
            {
                $cor_pattern = $cor_pattern[1];
            }
            else
                $cor_pattern = $cor_pattern[0];

            if(isset($betPrefixLen) && strpos($cor_pattern, $offerData['bets_prefix']) === 0){ $cor_pattern = substr ( $cor_pattern , $betPrefixLen );}
			
            $bet_title = '#'. (isset($odds['name']) ? $odds['name'] : $offerData['name']).'-'.$odds['tsid'].' '.$offerData['category'];
            $html .= '<td colspan="'.$colspan.'" class="option-'.(count($offerData['id']) > 3 ? 3 : count($offerData['odds'])).' '.($odds['odds'] > 1 ? ' betaction' : ' disabled').(strtolower($cor_pattern) == 'x' ? ' draw' : '') .'">'
                . OddsView::renderBtnBet($cor_pattern, $odds['odds'], (isset($odds['event_id']) ? $odds['event_id'] : $offerData['id']), $odds['id'], 'matchpage', $bet_title)
            .'</td>';
            $k++;
            if($k == 3) { break; }
        }
        print $html;
        ?> 
    </tr>
        <?php if($oddTotal > 3): ?>
            <tr class="altrow">
                <?php
                $html = '';
                $l = 0;
                $oddTotalLeft = $oddTotal - $k;
                $colspan = (6 / $oddTotalLeft);
                foreach($offerData['odds'] as $odds){
                    $l++;
                    if($l <= $k){ continue; }

                    $cor_pattern = explode('|', $odds['title']);
                    if(!$filtered && count($cor_pattern) > 1)
                    {
                        $cor_pattern = $cor_pattern[1];
                    }
                    else {
                        $cor_pattern = $cor_pattern[0];
                    }
                    $bet_title = '#'. (isset($odds['name']) ? $odds['name'] : $offerData['name']).'-'.$odds['tsid'].' '.$offerData['category'];

                    $html .= '<td colspan="'.$colspan.'" class="'.($odds['odds'] > 1 ? ' betaction' : ' betaction disabled').'">'
                                . OddsView::renderBtnBet($cor_pattern, $odds['odds'], (isset($odds['event_id']) ? $odds['event_id'] : $offerData['id']), $odds['id'], 'matchpage', $bet_title)
                            .'</td>';
                    $k++;
                }
                print $html;
                ?>
            </tr>
        <?php endif; ?>
<?php /* DEFAULT LIST STYLE */ else : ?>
    <?php
        $html = '';
        $k = 0;
        $has_draw = null;
        $colspan = ($oddTotal > 3) ? 2 : (6 / $oddTotal);
		
		if(isset($offerData['bets_prefix']))
		{			
			$html .= '<td class="betsPrefix">'.$offerData['bets_prefix'].'</td>';
			$colspan = ($oddTotal > 3) ? 3 : (6 / $oddTotal + 1);
			$betPrefixLen = strlen($offerData['bets_prefix']);
		}
		
		
        foreach($offerData['odds'] as $odds){            
            $cor_pattern = explode('|', $odds['title']);
            if( count($cor_pattern) > 1 && isset($betPrefixLen))
            {
                $cor_pattern = $cor_pattern[1];
            }			
            else{
                $cor_pattern = $cor_pattern[0];   
            }
            
            if(isset($betPrefixLen) && strpos($cor_pattern, $offerData['bets_prefix']) === 0){ 
                $_cor_pattern = substr ( $cor_pattern , $betPrefixLen ); 
                if(strlen($_cor_pattern)) {$cor_pattern = $_cor_pattern;}
            }
            $has_draw = strtolower($cor_pattern) == 'x' ? true : $has_draw;
			$bet_title = '#'. (isset($odds['name']) ? $odds['name'] : $offerData['name']).'-'.$odds['tsid'].' '.$offerData['category'];
            $html .= '<td colspan="'.$colspan.'" class="option-'.(count($offerData['id']) > 3 ? 3 : count($offerData['odds'])).' '.($odds['odds'] > 1 ? ' betaction' : ' betaction disabled').(strtolower($cor_pattern) == 'x' ? ' draw' : '') .'">'
                . OddsView::renderBtnBet($cor_pattern, $odds['odds'], (isset($odds['event_id']) ? $odds['event_id'] : $offerData['id']), $odds['id'], 'oddsList'.(isset($type) ? '-'.$type : ''), $bet_title)
            .'</td>';
            $k++;
            if($k == 3) { break; }            
        }       
        if($oddTotal > 3) {$offerData['count']++;}
    ?>
            
    <tr id="e_<?=$offerData['id']; ?>" class="<?=(count($offerData['odds']) > 1 ? "offers_line" : "").($has_draw ? ' has-draw' : '');?><?=(isset($type) ? ' event-filter-'.$type : '')?>">
        <td rowspan="<?=$rowspan;?>" class="date">            
            <?=OddsViewHelper::formatOddsDate($offerData['date'], false); ?>
            <?=odds_render_match_info($offerData['e_ext'], $offerData['aid'], $offerData['date'])?>
            <?php if(!OddsViewHelper::isLongTermOffer($offerData['name'])): ?><span class="name" style="display: none;">#<?=$offerData['name']; ?></span><?php endif;?>
            <?=OddsViewHelper::renderCombinationNotifications($offerData) ?> 
        </td>        
        <?=$html?>        
        <td rowspan="<?=$rowspan;?>" class="more"><?php if(!OddsViewHelper::isLongTermOffer($offerData['name'])):?><?= OddsViewHelper::linkMore($offerData['count'], $path.($offerData['pid']?$offerData['pid']:$offerData['id'])); ?><?php endif;?></td>
        <td class="stats-link"><?= odds_stats_icon( $offerData['pid'] != null ? $offerData['pid'] : $offerData['id'], $offerData['title']); ?></td>
    </tr>       
<?php endif; ?>