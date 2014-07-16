<div class="offer">
    <h4 id="odds-gt-<?=$gameType_id?>" class="odds-gametype" data-gtid="<?=$gameType_id?>" data-collapsed="0" title="<?=t('Paspauskite norint suskleisti/išskleisti statymo tipą')?>"><?=preg_replace('|{(?:[^}]*)}|', '', $title)?><span class="pull-right expanded-option"><?=t('Išskleisti')?></span></h4>
    <div class="offers_list-odds<?= ($gameTypeCollapsed && $eid ? ' collapse' : '')?>">
        
		<table class="odds<?php print (count($coefNames) == 1 ? ' offers_line' : ''); ?>">
			<tr>
				<th colspan="<?php print(!$loged?3:2)?>"></th>
				<?php
				foreach($coefNames as $name): ?>
				<th class="odd_title" title="<?=t('Baigtis '.$name)?>"><?php print $name; ?>
				</th>
				<?php endforeach; ?>
			</tr>
			<?php print $content; ?>
		</table>
		<div class="offers_list-rules">
		<?php if(isset($combtosamemodule) AND $combtosamemodule): ?>
		<?php print t('* - Įvykiai negali būti kombinuojami su ta pačia lyga').'<br/>'; ?>
		<?php endif; ?>
		<?php if(isset($combtosamearea) AND $combtosamearea): ?>
		<?php print t('** - Įvykiai negali būti kombinuojami su ta pačia sporto šaka ir lyga').'<br/>'; ?>
		<?php endif; ?>
		</div>
	</div>
</div>
