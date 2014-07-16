<?php 
    //droping {anytext} marking
    $title = preg_replace('|{(?:[^}]*)}|', '', $title);
    //$title = preg_replace('|\((.*)\)|', '<span class="scope">$1</span>', $title);
    $haystack = array(117,118);
    $class_offer = (in_array($gameType_id, $haystack) ? 'superbet' : '');
       
  
?>
<div class="offer <?=$class_offer?>">
    <h4 id="odds-gt-<?=$gameType_id?>" class="odds-gametype" data-gtid="<?=$gameType_id?>" data-collapsed="<?=(isset($gameTypeCollapsed) && $gameTypeCollapsed ? '1' : '0')?>" title="<?=t('Paspauskite norint suskleisti/išskleisti statymo tipą')?>"><?=$title?><span class="pull-right expanded-option"><?=t('Išskleisti')?></span></h4>
    <div class="offers_list-odds<?= (isset($gameTypeCollapsed) && $gameTypeCollapsed ? ' collapse' : ' expande')?>">
		<table class="odds">
			<?=$content; ?>
		</table>
		<div class="offers_list-rules">
		<?php if(isset($combtosamemodule) AND $combtosamemodule): ?>
		<?=t('* - Įvykiai negali būti kombinuojami su ta pačia lyga').'<br/>'; ?>
		<?php endif; ?>
		<?php if(isset($combtosamearea) AND $combtosamearea): ?>
		<?=t('** - Įvykiai negali būti kombinuojami su ta pačia sporto šaka ir lyga').'<br/>'; ?>
		<?php endif; ?>
		</div>
	</div>
</div>
