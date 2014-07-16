
(function ($) {

/**
 * Provide the summary information for the block settings vertical tab.
 */
Drupal.behaviors.topsportSettingsSummary = {
  attach: function (context) {
    console.log($('fieldset#edit-date_restriction', context));
    $('fieldset#edit-date-restriction', context).drupalSetSummary(function (context) {
      var summary = '';
      if ($('.form-item-date-restriction-enable input[type=checkbox]:checked', context).val()) {
        summary += Drupal.t('Rodyti tam tikru metu');
      }else {
        summary += Drupal.t('Not restricted');
      }
      return summary;
    });
  }
};

})(jQuery);
