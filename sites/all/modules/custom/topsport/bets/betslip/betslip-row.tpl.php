<div id="<?=$row_id?>" class="slip_row"  title="<?=$eventCat.t(', įvykio nr. @number', array('@number' => $eventName)); ?>">
	<div class="slip_row_data">
		<div class="row-left left">
			<div class="row-letter"
			<?php if($sliptype == 'system' AND $hideactions):?>
				style="display: block;" <?php endif; ?>>
				<?=$letter?>
			</div>
			<?php if(!$static):?>
			<div <?=($hideactions ? 'style="display: none;"' : '');?>
				class="row-selection selection <?=($selected ? 'selected' : 'unselected');?>">
				<?=l(' ', "betslip/edit/nojs/selection/$eid/$choise/".($selected ? 0 : 1), array('attributes' => array('class' => array('use-ajax'), 'title' => ($selected ? t('Atšaukti pasirinkimą') : t('Pasirinkti') ), 'data-toggle' => 'tooltip')));?>
			</div>
			<?php endif; ?>
			<?php if($sliptype == 'system'):?>
			<?php if(!$static):?>
            <div class="row-selection reorder" <?=($hideactions ? 'style="display: none;"' : '');?>>
				<a href="#" class="tabledrag-handle handle" data-toggle="tooltip" title="<?=t('Tempkite pertvarkymui'); ?>">reorder
				</a>
			</div>
			<?php endif; ?>
			<?php endif; ?>            
            <?php if(!$static):?>
                <div class="row-selection remove" <?=($hideactions ? 'style="display: none;"' : '');?>>
                    <?=l('delete', "betslip/edit/nojs/remove/$eid/$choise", array('attributes' => array('class' => array('use-ajax'), 'title' => t('Pašalinti'), 'data-toggle' => 'tooltip'))); ?>
                </div>
            <?php endif; ?>
		</div>
		<div class="row-right right">
			<div class="title">
                <strong><?=$title?></strong>
                <?php if($combcount):?>
                    <span data-toggle="tooltip" title="<?=t('Įvykis turi būti kombinuojamas su ne mažiau kaip @count įvykiais(-ių)', array('@count'=>$combcount-1))?>" class="label label-primary"><?=t('@count kom.', array('@count'=>$combcount))?></span>
                <?php endif; ?>
            </div>			
			<div class="bet-nr"><?='#'.$eventName; ?></div>
            <div class="type"><?=$type;?></div>
			<div class="choise"><?=t('Statymas');?>: <span class="selection"><?=$selection;?></span></div>
			<div class="rate"><?=$rate;?></div>
		</div>
	</div>	
</div>
