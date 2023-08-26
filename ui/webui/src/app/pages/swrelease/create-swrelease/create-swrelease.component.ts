import { Component } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';

@Component({
  selector: 'app-create-swrelease',
  templateUrl: './create-swrelease.component.html',
  styleUrls: ['./create-swrelease.component.scss']
})
export class CreateSwreleaseComponent {

  createSwReleaseForm: FormGroup;
  constructor(private fb: FormBuilder) {
    this.createSwReleaseForm = fb.group({
      fwreleasename:'',
      devicemodel:''
    });

  }

  onSubmit() {

  }
}
