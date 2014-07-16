<?php 
    $is_render = count($topbets) > 0;
?>
<?php if($is_render) :?>
    <div class="btn-group-supermulti"><span><?=t('Super kombinacija')?></span>
        <?php foreach ($topbets as $item):?>
                <button data-toggle="tooltip" title="<?=t('Super kombinacija')?>" onclick="Odds.setDailyComb('dc<?=$item['url']?>'); _gaq.push(['_trackEvent', 'SuggestedMiltiBet', 'SuperComb', '<?=$item['win']?>']);" class="btn btn-primary"><?=$item['win']. ' ' . $item['currency']?></button>
        <?php endforeach; ?>
    </div>
<?php endif ?>