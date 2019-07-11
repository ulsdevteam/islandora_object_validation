/**
 * @file
 * Javascript file for islandora_datastreams_io
 * 
 * The PHP code uses the constants for these 0, 1, 2, 3:
 *       ISLANDORA_OBJECT_VALIDATION_FETCH_SOLR = 0
 *       ISLANDORA_OBJECT_VALIDATION_FETCH_LISTPIDS = 1
 *       ISLANDORA_OBJECT_VALIDATION_FETCH_COLLECTION = 2
 *       ISLANDORA_OBJECT_VALIDATION_FETCH_MODEL = 3
 * 
 */

(function ($) {

    $(document).ready(function() {
        $('#edit-pids-fetch-method-0').change(function() {
            hideall_but(0);
        });
        $('#edit-pids-fetch-method-1').change(function() {
            hideall_but(1);
        });
        $('#edit-pids-fetch-method-2').change(function() {
            hideall_but(2);
        });
        $('#edit-pids-fetch-method-3').change(function() {
            hideall_but(3);
        });
        var i = $('input[name=pids_fetch_method]:checked', '#islandora-object-validation-validate-objects').val();
        hideall_but(i);
        $("#selected_dsid").hide();
        $('#edit-datastream').change(function(){
            var selected_text = $('#edit-datastream').find(":selected").text();
            //do something
            if (selected_text.toString().indexOf("*") > -1) {
              $("#edit-transform").attr('disabled',true);
            }
            else {
              $("#edit-transform").attr('disabled',false);
            }
            $('[name="selected_dsid"]').val(selected_text);
         });			
    });
  
    function hideall_but(i) {
        if (i == 1) {
            $('.form-item-solr-query').show();
            $('.form-item-list-of-pids').hide();
            $('.form-item-collection').hide();
            $('.form-item-model').hide();
        }
        if (i < 1) {
            $('.form-item-solr-query').hide();
            $('.form-item-list-of-pids').show();
            $('.form-item-collection').hide();
            $('.form-item-model').hide();
        }
        if (i == 2) {
            $('.form-item-solr-query').hide();
            $('.form-item-list-of-pids').hide();
            $('.form-item-collection').show();
            $('.form-item-model').hide();
        }
        if (i == 3) {
            $('.form-item-solr-query').hide();
            $('.form-item-list-of-pids').hide();
            $('.form-item-collection').hide();
            $('.form-item-model').show();
        }
    };
  
})(jQuery);
