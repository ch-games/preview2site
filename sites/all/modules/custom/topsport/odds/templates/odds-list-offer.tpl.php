<?php 
$offerData['category'] = htmlspecialchars($offerData['category']);
?>
<tr id="e_<?php print $offerData['id']; ?>"
	class="<?php print (count($offerData['odds']) > 1 ? "offers_line" : "");?>"
	title="<?php print $offerData['category'].t(', įvykio nr. @number', array('@number' => $offerData['name'])); ?>">
    <td class="date"><?php print offers_dateTimeFormat(strtotime($offerData['date'])); ?>
	</td>
	<?php if(isset($offerData['hidename']) && !$offerData['hidename']): ?>
	<td class="number"><?php print $offerData['name']; ?>
	</td>
	<?php endif; ?>
	<?php
	$html = '';
	$html .= '<td class="odd_title"> '.$offerData['title'].' </td>';
	foreach($offerData['odds'] as $odds){
		$html .= '<td id="'.'e_'.$offerData['id'].'_'.$odds['id'].'" '.($odds['odds'] > 1 ? 'onclick="_gaq.push([\'_trackEvent\', \'OddsClassic\', \''.$offerData['category'].'\', \'BetsSlip: '.$odds['id'].'\']); Drupal.betAction(this, '.$offerData['id'].', '.$odds['id'].');"' : '').' class="'.($odds['odds'] > 1 ? 'btn betaction' : 'disabled').'">'.($odds['odds'] > 1 ? '<span class="rate">'.$odds['odds'].'</span>' : '').'</td>';
	}
	print $html;
	?>    
	<td class="more<?=($eid ? ' empty' : '')?>" align="center">  <?php print  ($offerData['count'] ? l('+'.$offerData['count'], $path.($offerData['pid']?$offerData['pid']:$offerData['id'])) . odds_stats_icon( $offerData['id'], $offerData['title']) /*. offers_enetpulse_icon($offerData['#id'], 'node/'.$offerData['#nid']) */: ''); ?></td>
</tr>
