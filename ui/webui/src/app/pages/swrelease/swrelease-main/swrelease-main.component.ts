import { Component } from '@angular/core';

@Component({
  selector: 'app-swrelease-main',
  templateUrl: './swrelease-main.component.html',
  styleUrls: ['./swrelease-main.component.scss']
})
export class SwreleaseMainComponent {

  selectedItem:string = "CreateSWRelease";
  constructor() {

  }

  onClick(item:string) {
    this.selectedItem = item;
  }
}
