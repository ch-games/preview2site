(function ($) {
  function fixPaymentSum(){
    document.getElementById('paymentSum').value = document.getElementById('paymentSum').value.replace(",",".");
    document.getElementById('paymentSum').value = (document.getElementById('paymentSum').value / 1).toFixed(0);

    if (!parseFloat(document.getElementById('paymentSum').value)) {
      document.getElementById('paymentSum').value = '';
    }
  }
  
  function IsSupported(control, ver){
    return control.isVersionSupported(ver[0]+ "."+ ver[1] + "." + ver[2] + "." + ver[3]);
  }


})(jQuery);
