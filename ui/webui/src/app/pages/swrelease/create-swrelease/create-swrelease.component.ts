import { Component } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
//import { extract, pack, on } from 'tar-fs';
//import { fs } from 'fs';


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

  onChange(event:any) {
    /*
    const extractFilesDirs = extract(this.createSwReleaseForm.value.fwreleasename, {
      ignore (x:any, header:any) {
        // pass files & directories, ignore e.g. symlinks
        return header.type !== 'file' && header.type !== 'directory'
      }
    });

    console.log(extractFilesDirs);
    */
  }

  onSubmit() {
    
  }
}
